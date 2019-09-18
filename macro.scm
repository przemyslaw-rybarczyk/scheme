(define (ident? x)
  (or (symbol? x) (and (procedure? x) (pair? (x)))))

(define (eq-ident? x y)
  (if (procedure? x)
      (and (procedure? y)
           (let ((xr (x)) (yr (y)))
             (and (eq? (car xr) (car yr)) (= (cadr xr) (cadr yr)) (eq? (caddr xr) (caddr yr)))))
      (eq? x y)))

(define (equal-ident? x y)
  (cond ((pair? x)
         (and (pair? y) (equal-ident? (car x) (car y)) (equal-ident? (cdr x) (cdr y))))
        ((ident? x)
         (eq-ident? x y))
        (else
         (equal? x y))))

(define (memq-ident obj list)
  (cond ((null? list)
         #f)
        ((eq-ident? obj (car list))
         list)
        (else
         (memq-ident obj (cdr list)))))

(define (assq-ident obj list)
  (cond ((null? list)
         #f)
        ((eq-ident? obj (caar list))
         (car list))
        (else
         (assq-ident obj (cdr list)))))

;;; Each binding is of the form (name number-of-ellpises bindings).
;;; An identifier is a symbol or a nullary function returning (name macro-id env).

(define (get-empty-bindings pattern literals)
  (cond ((pair? pattern)
         (cond ((and (pair? (cdr pattern)) (eq? '... (cadr pattern)))
                (if (not (null? (cddr pattern)))
                    (error "Ellipsis not at the end of a list"))
                (map (lambda (binding) (list (car binding) (+ 1 (cadr binding)) (caddr binding)))
                     (get-empty-bindings (car pattern) literals)))
               (else
                (append (get-empty-bindings (car pattern) literals)
                        (get-empty-bindings (cdr pattern) literals)))))
        ((and (ident? pattern) (not (memq-ident pattern literals)))
         (list (list pattern 0 '())))
        (else
         '())))

(define (get-pattern-bindings expr pattern literals)
  (cond ((pair? pattern)
         (cond ((and (pair? (cdr pattern)) (eq? '... (cadr pattern)))
                (if (not (null? (cddr pattern)))
                    (error "Ellipsis not at the end of a list"))
                (let ((bindings-lists (map (lambda (subexpr) (get-pattern-bindings subexpr (car pattern) literals)) expr)))
                  (if (memq #f bindings-lists)
                      #f
                      (if (null? bindings-lists)
                          (get-empty-bindings pattern literals)
                          (apply
                            map
                            (lambda bindings (list (caar bindings) (+ 1 (cadar bindings)) (map caddr bindings)))
                            bindings-lists)))))
               ((pair? expr)
                (let ((car-bindings (get-pattern-bindings (car expr) (car pattern) literals))
                      (cdr-bindings (get-pattern-bindings (cdr expr) (cdr pattern) literals)))
                  (if (and car-bindings cdr-bindings)
                      (append car-bindings cdr-bindings)
                      #f)))
               (else
                #f)))
        ((and (ident? pattern) (not (memq-ident pattern literals)))
         (list (list pattern 0 expr)))
        ((equal-ident? expr pattern)
         '())
        (else
         #f)))

(define (filter f x)
  (if (null? x)
      '()
      (if (f (car x))
          (cons (car x) (filter f (cdr x)))
          (filter f (cdr x)))))

(define (flatten x)
  (if (pair? x)
      (append (flatten (car x)) (flatten (cdr x)))
      (list x)))

(define (used-bindings template bindings)
  (let ((flattened (filter ident? (flatten template))))
    (filter (lambda (binding) (memq (car binding) flattened)) bindings)))

(define (split-bindings bindings)
  (let ((vars (map car bindings))
        (levels (map cadr bindings))
        (vals (map caddr bindings)))
    (cond ((null? bindings)
           (error "Constant template followed by ellipsis"))
          ((memq #t (map (lambda (x) (= x 0)) levels))
           (error "Too many ellipses in template"))
          ((not (apply = (map length vals)))
           (error "Uneven ellipsis bindings"))
          (else
           (apply map (lambda sub-vals (map list vars (map (lambda (x) (- x 1)) levels) sub-vals)) vals)))))

(define (apply-pattern-bindings bindings template macro-id env)
  (cond ((pair? template)
         (if (and (pair? (cdr template)) (eq? '... (cadr template)))
             (let ((repeated-bindings (used-bindings template bindings)))
               (map (lambda (sub-bindings) (apply-pattern-bindings sub-bindings (car template) macro-id env))
                    (split-bindings repeated-bindings)))
             (cons (apply-pattern-bindings bindings (car template) macro-id env)
                   (apply-pattern-bindings bindings (cdr template) macro-id env))))
        ((eq? template '...)
         (error "Invalid ellipsis in pattern"))
        ((ident? template)
         (let ((binding (assq-ident template bindings)))
           (if binding
               (if (= (cadr binding) 0)
                   (caddr binding)
                   (error "Not enough ellipses in template"))
               (lambda () (list template macro-id env)))))
        (else
         template)))

(define macro-id-gen 0)

(define (new-macro-id)
  (set! macro-id-gen (+ macro-id-gen 1))
  macro-id-gen)

(define (apply-macro expr rules literals env)
  (if (null? rules)
      (error "Invalid macro expression - no patterns match")
      (let ((bindings (get-pattern-bindings expr (caar rules) literals)))
        (if bindings
            (apply-pattern-bindings bindings (cadar rules) (new-macro-id) env)
            (apply-macro expr (cdr rules) literals env)))))
