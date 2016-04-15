(define vsum (lambda (a b) (begin (print 'vsum a b nl)(+ a b))))
(define fibo 9999)
(define f
  (lambda (n)
    (if (< n 2)
	n
      (+ (fibo (- n 1)) (fibo (- n 2))))))
(set! fibo f)
(define n 0)
(while (<= n 10) (begin (print n (fibo n) nl) (set! n (+ n 1))))
