;
; test lisp expressions					;
0
112
-17
(+ 2 3)
(* 2 3)
(- 2 3)
(/ 2 3)
'(a b c)
'(a () c)
'( () () () )
; test defines
(define x 2)
x
(set! x 3)
x
; test functions
(define f (lambda (y) (+ x y)))
(f 7) ; should produce 10
(define x 4)
(f -1) ; should produce 2
;;; test macros
(define a 2)
(define b 3)
(define foo (lambda x x))
(define bar (lambda 'x x))
(define baz (lambda (x 'y) (print x y nl)))
(foo a b)
(bar a b)
(baz a b)
(if (< a b) "yes" "no")
(if (< b a) "yes" "no")
