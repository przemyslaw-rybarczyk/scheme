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
(define period 46)

(define (parse-and-compile)
  (compile-top-level (parse) #t))

(define (parse)
  (let ((token (read-token)))
    (if (pair? token)
        (let ((id (cdr token)))
          (cond ((= id left-paren)
                 (parse-list))
                ((= id right-paren)
                 (error "Syntax error: unexpected ')'"))
                ((= id quote-char)
                 (list 'quote (parse)))
                ((= id period)
                 (error "Syntax error: unexpected '.'"))))
        token)))

(define (parse-list)
  (let ((token (peek-token)))
    (cond ((equal? token (cons 'token right-paren))
           (read-token)
           '())
          ((equal? token (cons 'token period))
           (read-token)
           (let ((cdr-expr (parse)))
             (if (not (equal? (read-token) (cons 'token right-paren)))
                 (error "Syntax error: expected ')'"))
             cdr-expr))
          (else
           (let ((head (parse)))
             (let ((tail (parse-list)))
               (cons head tail)))))))

(define (compile-top-level expr tail)
  (let ((expanded (expand-to-define expr '())))
    (if (pair? expanded)
        (cond ((eq? 'define (car expanded))
               (let ((expr (transform-define expanded)))
                 (compile (caddr expr) '() #f)
                 (set-def! (next-inst) (cadr expr))
                 (put-tail! tail)))
              ((eq? 'begin (car expanded))
               (validate-expr expanded '((begin expr ...)) '())
               (compile-top-level-seq (cdr expanded) tail))
              ((eq? 'define-syntax (car expanded))
               (validate-expr expanded '((define-syntax keyword (syntax-rules literals (pattern template) ...))) '(syntax-rules))
               (validate-syntax-binding (cdr expanded))
               (set! macros (cons (list (cadr expanded)
                                        (cadr (caddr expanded))
                                        (map (lambda (rule) (list (cdar rule) (cadr rule)))
                                             (cddr (caddr expanded)))
                                        '())
                                  macros))
               (set-const! (next-inst) #!void)
               (put-tail! tail))
              (else
               (compile expanded '() tail)))
        (compile expr '() tail))))

(define (compile-top-level-seq exprs tail)
  (cond ((null? exprs)
         (set-const! (next-inst) #!void)
         (put-tail! tail))
        ((null? (cdr exprs))
         (compile-top-level (car exprs) tail))
        (else
         (compile-top-level (car exprs) #f)
         (set-delete! (next-inst))
         (compile-top-level-seq (cdr exprs) tail))))

(define (compile expr env tail)
  (cond ((pair? expr)
         (let ((loc (locate-name (car expr) env)))
           (if (pair? loc)
               (if (eq? (car loc) 'macro)
                   (compile (apply-macro (cdr expr) (cadr loc) (caddr loc) (cadddr loc)) env tail)
                   (compile-appl expr env tail))
               (let ((macro (assq-ident loc macros)))
                 (if macro
                     (compile (apply-macro (cdr expr) (cadr macro) (caddr macro) '()) env tail)
                     (let ((derived-form (assq-ident loc derived-forms)))
                       (if derived-form
                           (compile (apply-macro (cdr expr) (cadr derived-form) (caddr derived-form) (cadddr derived-form)) env tail)
                           (let ((primitive-form (assq-ident loc primitive-forms)))
                             (if primitive-form
                                 ((cadr primitive-form) expr env tail)
                                 (compile-appl expr env tail))))))))))
        ((ident? expr)
         (compile-name expr env tail))
        ((null? expr)
         (error "() is not a valid expression"))
        (else
         (set-const! (next-inst) expr)
         (put-tail! tail))))

;;; Returns define, begin or define-syntax expression if expr expands to either of these, and #f otherwise.
(define (expand-to-define expr env)
  (if (pair? expr)
      (let ((loc (locate-name (car expr) env)))
        (if (pair? loc)
            (if (eq? (car loc) 'macro)
                (expand-to-define (apply-macro (cdr expr) (cadr loc) (caddr loc) (cadddr loc)) env)
                #f)
            (let ((macro (assq-ident loc macros)))
              (if macro
                  (expand-to-define (apply-macro (cdr expr) (cadr macro) (caddr macro) '()) env)
                  (if (or (eq? loc 'define) (eq? loc 'begin) (eq? loc 'define-syntax))
                      (cons loc (cdr expr))
                      #f)))))
      #f))

(define (compile-name name env tail)
  (let ((loc (locate-name name env)))
    (if (pair? loc)
        (if (eq? 'macro (car loc))
            (error "Use of macro as variable")
            (set-var! (next-inst) (car loc) (cdr loc)))
        (if (assq-ident loc macros)
            (error "Use of macro as variable")
            (set-name! (next-inst) loc)))
    (put-tail! tail)))

(define (env-distance env1 env2)
  (define (loop env1 d)
    (if (and (pair? env1) (eq? (caar env1) 'macro))
        (loop (cdr env1) d)
        (if (eq? env1 env2)
            d
            (loop (cdr env1) (+ d 1)))))
  (if (and (pair? env2) (eq? (caar env2) 'macro))
      (env-distance env1 (cdr env2))
      (loop env1 0)))

;;; Returns (frame . index) if a local variable is found,
;;; ('macro . macro) if a local macro is found, and name as a symbol otherwise.
(define (locate-name name env0)
  (define (loop-env name env x)
    (define (loop-var-frame frame y)
      (cond ((null? frame)
             (loop-env name (cdr env) (+ x 1)))
            ((eq-ident? (car frame) name)
             (cons x y))
            (else
             (loop-var-frame (cdr frame) (+ y 1)))))
    (define (loop-macro-frame frame)
      (cond ((null? frame)
             (loop-env name (cdr env) x))
            ((eq-ident? (caar frame) name)
             (cons 'macro (cdar frame)))
            (else
             (loop-macro-frame (cdr frame)))))
    (cond ((null? env)
           (if (procedure? name)
               (let ((new-name (car (name)))
                     (new-env (caddr (name))))
                 (loop-env new-name new-env (env-distance env0 new-env)))
               name))
          ((eq? (caar env) 'var)
           (loop-var-frame (cdar env) 0))
          ((eq? (caar env) 'macro)
           (loop-macro-frame (cdar env)))
          (else
           (error "Invalid environment structure"))))
  (loop-env name env0 0))

(define (compile-appl expr env tail)
  (validate-expr expr '((f x ...)) '())
  (for-each
    (lambda (expr) (compile expr env #f))
    expr)
  ((if tail set-tail-call! set-call!)
   (next-inst) (- (length expr) 1)))

(define (compile-set expr env tail)
  (validate-expr expr '((set! name expr)) '())
  (if (not (null? (cdddr expr)))
      (error "set! expression too long"))
  (compile (caddr expr) env #f)
  (let ((loc (locate-name (cadr expr) env)))
    (if (pair? loc)
        (if (eq? 'macro (car loc))
            (error "Use of macro as variable")
            (set-set! (next-inst) (car loc) (cdr loc)))
        (if (assq-ident loc macros)
            (error "Use of macro as variable")
            (set-set-name! (next-inst) loc)))
    (put-tail! tail)))

(define (compile-if expr env tail)
  (validate-expr expr '((if pred conseq) (if pred conseq alter)) '())
  (compile (cadr expr) env #f)
  (let ((jump-false (next-inst)))
    (compile (caddr expr) env tail)
    (let ((jump-true (next-inst)))
      (set-jump-false! jump-false (this-inst))
      (if (null? (cdddr expr))
          (begin
            (set-const! (next-inst) #!void)
            (put-tail! tail))
          (compile (cadddr expr) env tail))
      (set-jump! jump-true (this-inst)))))

(define (has-duplicates? list)
  (cond ((null? list)
         #f)
        ((memq-ident (car list) (cdr list))
         #t)
        (else
         (has-duplicates? (cdr list)))))

(define (transform-variadic x)
  (cond ((pair? x)
         (cons (car x) (transform-variadic (cdr x))))
        ((null? x)
         '())
        (else
         (list x))))

(define (compile-lambda expr env tail)
  (validate-expr expr '((lambda params expr ...)) '())
  (let ((args (transform-variadic (cadr expr))))
    (if (memq #f (map ident? args))
        (error "lambda parameter is not a variable name"))
    (if (has-duplicates? args)
        (error "duplicate lambda parameter name"))
    (let ((jump-after (next-inst))
          (lambda-address (this-inst)))
      (compile-body (cddr expr) (cons (cons 'var args) env) #t)
      (let ((lambda-inst (next-inst))
            (variadic (not (equal-ident? args (cadr expr)))))
        (set-lambda!
          lambda-inst
          variadic
          (if variadic
              (- (length args) 1)
              (length args))
          lambda-address)
        (set-jump! jump-after lambda-inst)
        (put-tail! tail)))))

(define (compile-begin expr env tail)
  (validate-expr expr '((begin expr ...)) '())
  (compile-seq (cdr expr) env tail))

(define (compile-quote expr env tail)
  (validate-expr expr '((quote expr)) '())
  (compile-quoted (cadr expr))
  (put-tail! tail))

(define (compile-quoted expr)
  (cond ((pair? expr)
         (compile-quoted (car expr))
         (compile-quoted (cdr expr))
         (set-cons! (next-inst)))
        (else
         (set-const! (next-inst) (reduce-ident expr)))))

(define (validate-syntax-binding expr)
  (if (not (ident? (car expr)))
      (error "Syntax binding keyword is not an identifier"))
  (let ((transformer (cadr expr)))
    (if (memq #f (map ident? (cadr transformer)))
        (error "Transformer literals are not a list of identifiers"))
    (if (memq #f (map (lambda (rule) (eq-ident? (caar rule) (car expr))) (cddr transformer)))
        (error "Syntax rule pattern doesn't start with bound symbol"))
    (if (memq #t (map (lambda (rule)
                        (has-duplicates? (filter (lambda (symbol) (and (ident? symbol)
                                                                       (not (memq-ident symbol (cadr transformer)))
                                                                       (not (eq? '... (reduce-ident symbol)))))
                                                 (flatten (cdar rule)))))
                      (cddr transformer)))
        (error "Syntax rule pattern contains duplicates"))))

;;; Each local macro is of the form (name literals rules env).

(define (compile-let-syntax expr env tail)
  (validate-expr expr '((let-syntax ((keyword (syntax-rules literals (pattern template) ...)) ...) expr ...)) '(syntax-rules))
  (for-each validate-syntax-binding (cadr expr))
  (compile-body
    (cddr expr)
    (cons (cons 'macro (map (lambda (binding) (list (car binding)
                                                    (cadadr binding)
                                                    (map (lambda (rule) (list (cdar rule) (cadr rule))) (cddadr binding))
                                                    env))
                            (cadr expr)))
          env)
    tail))

(define (compile-letrec-syntax expr env tail)
  (validate-expr expr '((letrec-syntax ((keyword (syntax-rules literals (pattern template) ...)) ...) expr ...)) '(syntax-rules))
  (if (not (list? (cadr expr)))
      (error "Syntax bindings are not a list"))
  (for-each validate-syntax-binding (cadr expr))
  (let ((new-env (cons '() env)))
    (set-car! new-env
              (cons 'macro (map (lambda (binding) (list (car binding)
                                                        (cadadr binding)
                                                        (map (lambda (rule) (list (cdar rule) (cadr rule))) (cddadr binding))
                                                        new-env))
                                (cadr expr))))
    (compile-body (cddr expr) new-env tail)))

(define (compile-seq exprs env tail)
  (cond ((null? exprs)
         (set-const! (next-inst) #!void)
         (put-tail! tail))
        ((null? (cdr exprs))
         (compile (car exprs) env tail))
        (else
         (compile (car exprs) env #f)
         (set-delete! (next-inst))
         (compile-seq (cdr exprs) env tail))))

(define (transform-define expr)
  (validate-expr expr '((define var val) (define (var . params) expr ...)) '())
  (if (pair? (cadr expr))
      (list 'define (caadr expr) (cons 'lambda (cons (cdadr expr) (cddr expr))))
      expr))

(define (split-defines exprs env)
  (if (null? exprs)
      (cons '() '())
      (let ((expanded (expand-to-define (car exprs) env)))
        (if (pair? expanded)
            (cond ((eq? (car expanded) 'define)
                   (let ((x (split-defines (cdr exprs) env)))
                     (cons (cons expanded (car x)) (cdr x))))
                  ((eq? (car expanded) 'begin)
                   (validate-expr expanded '((begin expr ...)) '())
                   (split-defines (append (cdr expanded) (cdr exprs)) env))
                  (else
                   (cons '() exprs)))
            (cons '() exprs)))))

(define (compile-body exprs env tail)
  (let ((x (split-defines exprs env)))
    (let ((defines (car x))
          (seq (cdr x)))
      (if (null? defines)
          (compile-seq seq env tail) ; prevent infinite loop
          (compile (cons 'letrec (cons (map cdr (map transform-define defines)) seq)) env tail)))))

(define (put-tail! tail)
  (if tail
      (set-return! (next-inst))))

(define (error-define expr env tail)
  (error "Invalid use of define"))

(define (error-define-syntax expr env tail)
  (error "Invalid use of define-syntax"))

(define primitive-forms
  (list
    (list 'set! compile-set)
    (list 'if compile-if)
    (list 'lambda compile-lambda)
    (list 'begin compile-begin)
    (list 'quote compile-quote)
    (list 'let-syntax compile-let-syntax)
    (list 'letrec-syntax compile-letrec-syntax)
    (list 'define error-define)
    (list 'define-syntax error-define-syntax)))

(define derived-forms
  '((let ()
      (((((var val) ...) expr ...)
        ((lambda (var ...) expr ...) val ...))))
    (letrec ()
      (((((var val) ...) expr ...)
        (letrec-syntax (... ((letrec2 (syntax-rules ()
                                        ((letrec2 () (temp ...) ((var2 val2) ...) (expr2 ...))
                                         (let ((var2 #!undef) ...)
                                           (let ((temp val2) ...)
                                             (set! var2 temp) ...
                                             expr2 ...)))
                                        ((letrec2 (x y ...) (temp ...) bindings exprs)
                                         (letrec2 (y ...) (new_temp temp ...) bindings exprs))))))
          (letrec2 (var ...) () ((var val) ...) (expr ...))))))
    (cond (else)
      ((()
        #!void)
       (((else expr ...))
        (begin expr ...))
       (((test expr ...) clause ...)
        (if test
            (begin expr ...)
            (cond clause ...)))))
    (and ()
      ((()
        #t)
       ((expr)
        expr)
       ((expr1 expr2 ...)
        (if expr1
            (and expr2 ...)
            #f))))
    (or ()
      ((()
        #f)
       ((expr)
        expr)
       ((expr1 expr2 ...)
        (let ((x expr1))
          (if x x (or expr2 ...))))))))

(for-each
  (lambda (form) (set-cdr! (cddr form) (list (list (cons 'macro derived-forms)))))
  derived-forms)

(define macros '())
