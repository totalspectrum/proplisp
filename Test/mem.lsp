;;
;; this is a program that uses lots of memory if we are not
;; garbage collecting, but should be harmless if we are
;;
(define n 0)
(while (< n 10000) (begin (set! n (+ n 1))))
n
