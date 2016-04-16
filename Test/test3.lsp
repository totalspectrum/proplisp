;; test for variable binding
(define x 99)
(define f (lambda (y) (begin (define x 1)(set! y (+ y x))(set! x 7) y)))
x
(f 2)
x

;; now check for redefines
(set! x 88)
(define catchx (lambda () x))
(catchx)   ;; should return 88
(set! x 10) ;; 10
(catchx)    ;; 10
(define x 5) ;; 5
(catchx)     ;; 5
(set! x 4)   ;; 4
(catchx)     ;; 4
'done
