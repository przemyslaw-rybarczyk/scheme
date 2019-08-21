;;; replacements for stdlib

(define (append x y)
  (if (null? x)
      y
      (cons (car x) (append (cdr x) y))))

(define (map f x)
  (if (null? x)
      '()
      (cons (f (car x)) (map f (cdr x)))))

(define (map2 f x y)
  (if (or (null? x) (null? y))
      '()
      (cons (f (car x) (car y)) (map2 f (cdr x) (cdr y)))))

(define (for-each f x)
  (if (null? x)
      #!void
      (begin (f (car x))
             (for-each f (cdr x)))))

(define (assoc obj alist)
  (cond ((null? alist)
         #f)
        ((equal? obj (caar alist))
         (car alist))
        (else
         (assoc obj (cdr alist)))))

;;; proper compiler

(define token-buffer '())

(define (read-token)
  (if (eq? token-buffer '())
      (next-token)
      (let ((token token-buffer))
        (set! token-buffer '())
        token)))

(define (peek-token)
  (if (eq? token-buffer '())
      (set! token-buffer (next-token)))
  token-buffer)

(define quote-char 39)
(define left-paren 40)
(define right-paren 41)

(define (parse-and-compile)
  (compile-top-level (parse) '() builtin-forms #t))

(define (parse)
  (let ((token (read-token)))
    (if (pair? token)
        (let ((id (cdr token)))
          (cond ((= id left-paren)
                 (parse-list))
                ((= id right-paren)
                 (error "Syntax error: unexpected ')'"))
                ((= id quote-char)
                 (list 'quote (parse)))))
        token)))

(define (parse-list)
  (if (equal? (cons 'token right-paren) (peek-token))
      (begin (read-token) '())
      (let ((head (parse)))
        (let ((tail (parse-list)))
          (cons head tail)))))

(define undef +)

(define (compile-top-level expr env forms tail)
  (cond ((and (pair? expr) (eq? 'define (car expr)))
         (compile-define expr env forms tail))
        ((and (pair? expr) (eq? 'begin (car expr)))
         (cond ((null? (cdr expr))
                (put-tail! tail))
               ((null? (cddr expr))
                (compile-top-level (cadr expr) env forms tail)
                (put-tail! tail))
               (else
                (compile-top-level (cadr expr) env forms #f)
                (set-delete! (next-inst))
                (compile-top-level (cons 'begin (cddr expr)) env forms tail))))
        (else
         (compile expr env forms tail))))

(define (compile-define expr env forms tail)
  (let ((expr (transform-define expr)))
    (compile (caddr expr) env forms #f)
    (set-def! (next-inst) (cadr expr))
    (put-tail! tail)))

(define (compile expr env forms tail)
  (cond ((pair? expr)
         (let ((compile-form (assoc (car expr) forms)))
           (if compile-form
               (let ((type (cadr compile-form))
                     (f (caddr compile-form)))
                 (cond ((eq? type 'prim)
                        (f expr env forms tail))
                       ((eq? type 'deriv)
                        (compile (f expr) env forms tail))
                       (else
                        (error "Compiler error: unknown primitive type"))))
               (compile-appl expr env forms tail))))
        ((symbol? expr)
         (compile-name expr env tail))
        ((procedure? expr)
         (set-const! (next-inst) #!undef)
         (put-tail! tail))
        (else
         (set-const! (next-inst) expr)
         (put-tail! tail))))

(define (compile-name name env tail)
  (let ((loc (locate-name name env)))
    (if loc
        (set-var! (next-inst) (car loc) (cdr loc))
        (set-name! (next-inst) name))
    (put-tail! tail)))

(define (locate-name name env)
  (define (loop-env env x)
    (define (loop-frame frame y)
      (cond ((null? frame)
             (loop-env (cdr env) (+ x 1)))
            ((equal? (car frame) name)
             (cons x y))
            (else
             (loop-frame (cdr frame) (+ y 1)))))
    (if (null? env)
        #f
        (loop-frame (car env) 0)))
  (loop-env env 0))

(define (compile-appl expr env forms tail)
  (for-each
    (lambda (expr) (compile expr env forms #f))
    expr)
  ((if tail set-tail-call! set-call!)
   (next-inst) (- (length expr) 1)))

(define (compile-set expr env forms tail)
  (compile (caddr expr) env forms #f)
  (let ((loc (locate-name (cadr expr) env)))
    (if loc
        (set-set! (next-inst) (car loc) (cdr loc))
        (set-set-name! (next-inst) (cadr expr)))
    (put-tail! tail)))

(define (compile-if expr env forms tail)
  (compile (cadr expr) env forms #f)
  (let ((jump-false (next-inst)))
    (compile (caddr expr) env forms tail)
    (let ((jump-true (next-inst)))
      (set-jump-false! jump-false (this-inst))
      (if (null? (cdddr expr))
          (begin
            (set-const! (next-inst) #!void)
            (put-tail! tail))
          (compile (cadddr expr) env forms tail))
      (set-jump! jump-true (this-inst)))))

(define (compile-lambda expr env forms tail)
  (let ((jump-after (next-inst))
        (lambda-address (this-inst)))
    (compile-body (cddr expr) (cons (cadr expr) env) forms #t)
    (let ((lambda-inst (next-inst)))
      (set-lambda! lambda-inst (length (cadr expr)) lambda-address)
      (set-jump! jump-after lambda-inst)
      (put-tail! tail))))

(define (compile-begin expr env forms tail)
  (compile-seq (cdr expr) env forms tail))

(define (compile-quote expr env forms tail)
  (compile-quoted (cadr expr))
  (put-tail! tail))

(define (compile-quoted expr)
  (if (pair? expr)
      (begin
        (compile-quoted (car expr))
        (compile-quoted (cdr expr))
        (set-cons! (next-inst)))
      (set-const! (next-inst) expr)))

(define (transform-let expr)
  (cons (cons 'lambda (cons (map car (cadr expr)) (cddr expr))) (map cadr (cadr expr))))

(define (transform-letrec expr)
  (list 'let
        (map (lambda (x) (list (car x) undef)) (cadr expr))
        (let ((new-symbols (map (lambda (x) (new-symbol)) (cadr expr))))
          (append (list 'let (map2 (lambda (x y) (list x (cadr y))) new-symbols (cadr expr)))
                  (append (map2 (lambda (x y) (list 'set! (car x) y)) (cadr expr) new-symbols)
                          (cddr expr))))))

(define (transform-cond expr)
  (cond ((null? (cdr expr))
         #f)
        ((eq? (caadr expr) 'else)
         (cons 'begin (cdadr expr)))
        (else
         (list 'if (caadr expr) (cons 'begin (cdadr expr)) (cons 'cond (cddr expr))))))

(define (transform-and expr)
  (cond ((null? (cdr expr))
         #f)
        ((null? (cddr expr))
         (cadr expr))
        (else
         (list 'if (cadr expr) (cons 'and (cddr expr)) #f))))

(define (transform-or expr)
  (cond ((null? (cdr expr))
         #t)
        ((null? (cddr expr))
         (cadr expr))
        (else
          (let ((v (new-symbol)))
            (list 'let (list (list v (cadr expr))) (list 'if v v (cons 'or (cddr expr))))))))

(define (compile-seq exprs env forms tail)
  (cond ((null? exprs)
         (put-tail! tail))
        ((null? (cdr exprs))
         (compile (car exprs) env forms tail))
        (else
         (compile (car exprs) env forms #f)
         (set-delete! (next-inst))
         (compile-seq (cdr exprs) env forms tail))))

(define (transform-define expr)
  (if (pair? (cadr expr))
      (list 'define (caadr expr) (cons 'lambda (cons (cdadr expr) (cddr expr))))
      expr))

(define (split-defines exprs)
  (cond ((null? exprs)
         (cons '() '()))
        ((eq? 'define (caar exprs))
         (let ((x (split-defines (cdr exprs))))
           (cons (cons (car exprs) (car x)) (cdr x))))
        ((eq? 'begin (caar exprs))
         (split-defines (append (cdar exprs) (cdr exprs))))
        (else
         (cons '() exprs))))

(define (compile-body exprs env forms tail)
  (let ((x (split-defines exprs)))
    (let ((defines (car x))
          (seq (cdr x)))
      (if (null? defines)
          (compile-seq seq env forms tail) ; prevent infinite loop
          (compile (cons 'letrec (cons (map cdr (map transform-define defines)) seq)) env forms tail)))))

(define (put-tail! tail)
  (if tail
      (set-return! (next-inst))))

(define builtin-forms
  (list
    (list 'set! 'prim compile-set)
    (list 'if 'prim compile-if)
    (list 'lambda 'prim compile-lambda)
    (list 'begin 'prim compile-begin)
    (list 'quote 'prim compile-quote)
    (list 'let 'deriv transform-let)
    (list 'letrec 'deriv transform-letrec)
    (list 'cond 'deriv transform-cond)
    (list 'and 'deriv transform-and)
    (list 'or 'deriv transform-or)))
