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
  (compile-top-level (parse) '() #t))

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

(define undef +)

(define (compile-top-level expr env tail)
  (cond ((and (pair? expr) (eq? 'define (car expr)))
         (compile-define expr env tail))
        ((and (pair? expr) (eq? 'begin (car expr)))
         (cond ((null? (cdr expr))
                (set-const! (next-inst) #!void)
                (put-tail! tail))
               ((null? (cddr expr))
                (compile-top-level (cadr expr) env tail)
                (put-tail! tail))
               (else
                (compile-top-level (cadr expr) env #f)
                (set-delete! (next-inst))
                (compile-top-level (cons 'begin (cddr expr)) env tail))))
        ((and (pair? expr) (eq? 'define-syntax (car expr)))
         (set! forms (cons (list (cadr expr) 'macro (list (cddr (caddr expr)) (cadr (caddr expr)))) forms))
         (set-const! (next-inst) #!void)
         (put-tail! tail))
        (else
         (compile expr env tail))))

(define (compile-define expr env tail)
  (let ((expr (transform-define expr)))
    (compile (caddr expr) env #f)
    (set-def! (next-inst) (cadr expr))
    (put-tail! tail)))

(define (compile expr env tail)
  (cond ((pair? expr)
         (let ((compile-form (assq-ident (car expr) forms)))
           (if compile-form
               (let ((type (cadr compile-form))
                     (f (caddr compile-form)))
                 (cond ((eq? type 'prim)
                        (f expr env tail))
                       ((eq? type 'deriv)
                        (compile (f expr) env tail))
                       ((eq? type 'macro)
                        (compile (apply-macro expr (car f) (cadr f) '()) env tail))
                       (else
                        (error "Internal compiler error: unknown primitive type"))))
               (compile-appl expr env tail))))
        ((ident? expr)
         (compile-name expr env tail))
        ((eq? expr undef)
         (set-const! (next-inst) #!undef)
         (put-tail! tail))
        ((null? expr)
         (error "() is not a valid expression"))
        (else
         (set-const! (next-inst) expr)
         (put-tail! tail))))

(define (compile-name name env tail)
  (let ((loc (locate-name name env)))
    (if loc
        (set-var! (next-inst) (car loc) (cdr loc))
        (set-name! (next-inst) (if (symbol? name) name (car (name)))))
    (put-tail! tail)))

(define (locate-name name env)
  (define (loop-env env x)
    (define (loop-frame frame y)
      (cond ((null? frame)
             (loop-env (cdr env) (+ x 1)))
            ((eq-ident? (car frame) name)
             (cons x y))
            (else
             (loop-frame (cdr frame) (+ y 1)))))
    (if (null? env)
        #f
        (loop-frame (car env) 0)))
  (loop-env env 0))

(define (compile-appl expr env tail)
  (for-each
    (lambda (expr) (compile expr env #f))
    expr)
  ((if tail set-tail-call! set-call!)
   (next-inst) (- (length expr) 1)))

(define (compile-set expr env tail)
  (if (not (null? (cdddr expr)))
      (error "set! expression too long"))
  (compile (caddr expr) env #f)
  (let ((loc (locate-name (cadr expr) env)))
    (if loc
        (set-set! (next-inst) (car loc) (cdr loc))
        (set-set-name! (next-inst) (cadr expr)))
    (put-tail! tail)))

(define (compile-if expr env tail)
  (if (and (not (null? (cdddr expr))) (not (null? (cddddr expr))))
      (error "if expression too long"))
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
  (let ((args (transform-variadic (cadr expr))))
    (if (memq #f (map ident? args))
        (error "lambda parameter is not a variable name"))
    (if (has-duplicates? args)
        (error "duplicate lambda parameter name"))
    (let ((jump-after (next-inst))
          (lambda-address (this-inst)))
      (compile-body (cddr expr) (cons args env) #t)
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
  (compile-seq (cdr expr) env tail))

(define (compile-quote expr env tail)
  (if (not (null? (cddr expr)))
      (error "quote expression too long"))
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
  (if (memq #f (map null? (map cddr (cadr expr))))
      (error "invalid let expression binding"))
  (cons (cons 'lambda (cons (map car (cadr expr)) (cddr expr))) (map cadr (cadr expr))))

(define (transform-letrec expr)
  (if (memq #f (map null? (map cddr (cadr expr))))
      (error "invalid letrec expression binding"))
  (list 'let
        (map (lambda (x) (list (car x) undef)) (cadr expr))
        (let ((new-symbols (map (lambda (x) (new-symbol)) (cadr expr))))
          (append (list 'let (map (lambda (x y) (list x (cadr y))) new-symbols (cadr expr)))
                  (append (map (lambda (x y) (list 'set! (car x) y)) (cadr expr) new-symbols)
                          (cddr expr))))))

(define (transform-cond expr)
  (cond ((null? (cdr expr))
         #!void)
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
  (if (pair? (cadr expr))
      (list 'define (caadr expr) (cons 'lambda (cons (cdadr expr) (cddr expr))))
      (begin
        (if (not (null? (cdddr expr)))
            (error "define expression too long"))
        expr)))

(define (split-defines exprs)
  (cond ((null? exprs)
         (cons '() '()))
        ((not (pair? (car exprs)))
         (cons '() exprs))
        ((eq? 'define (caar exprs))
         (let ((x (split-defines (cdr exprs))))
           (cons (cons (car exprs) (car x)) (cdr x))))
        ((eq? 'begin (caar exprs))
         (split-defines (append (cdar exprs) (cdr exprs))))
        (else
         (cons '() exprs))))

(define (compile-body exprs env tail)
  (let ((x (split-defines exprs)))
    (let ((defines (car x))
          (seq (cdr x)))
      (if (null? defines)
          (compile-seq seq env tail) ; prevent infinite loop
          (compile (cons 'letrec (cons (map cdr (map transform-define defines)) seq)) env tail)))))

(define (put-tail! tail)
  (if tail
      (set-return! (next-inst))))

(define forms
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
