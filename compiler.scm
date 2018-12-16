;;; replacements for stdlib

(define (for-each f x)
  (if (null? x)
      #f ; TODO void
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

(define quote-char 39)
(define left-paren 40)
(define right-paren 41)

(define (parse-and-compile)
  (compile (parse) '() builtin-forms #t))

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

(define (compile expr env forms tail)
  (cond ((pair? expr)
         (let ((compile-form (assoc (car expr) forms)))
           (if compile-form
               ((cdr compile-form) expr env forms tail)
               (compile-appl expr env forms tail))))
        ((symbol? expr)
         (compile-name expr env tail))
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
  (if tail
      (set-tail-call! (next-inst) (- (length expr) 1))
      (set-call! (next-inst) (- (length expr) 1))))

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
            (set-const! (next-inst) #f) ; #!void
            (put-tail! tail))
          (compile (cadddr expr) env forms tail))
      (set-jump! jump-true (this-inst)))))

(define (compile-lambda expr env forms tail)
  (let ((jump-after (next-inst))
        (lambda-address (this-inst)))
    (compile-seq (cddr expr) (cons (cadr expr) env) forms #t)
    (let ((lambda-inst (next-inst)))
      (set-lambda! lambda-inst (length (cadr expr)) lambda-address)
      (set-jump! jump-after lambda-inst)
      (put-tail! tail))))

(define (compile-begin expr env forms tail)
  (compile-seq (cdr expr) env forms tail))

(define builtin-forms
  (list
    (cons 'set! compile-set)
    (cons 'if compile-if)
    (cons 'lambda compile-lambda)
    (cons 'begin compile-begin)))

(define (compile-seq exprs env forms tail)
  (cond ((null? exprs)
         (put-tail! tail))
        ((null? (cdr exprs))
         (compile (car exprs) env forms tail))
        (else
         (compile (car exprs) env forms #f)
         (set-delete! (next-inst))
         (compile-seq (cdr exprs) env forms tail))))

(define (put-tail! tail)
  (if tail
      (set-return! (next-inst))))
