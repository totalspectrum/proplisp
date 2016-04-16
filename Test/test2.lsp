;;
;; test function definitions
;;
(define x 99)
(define body 1)  ; to check for confusion

(define list (lambda x x))
(list 1 2 3)

;; we want (defun name args body) -> (define name (lambda args body))
(define defun
  (lambda ('name 'args 'body)
    (list 'define name (list 'lambda args body))))

(define a (defun getx () x))
(eval a)
(getx)  ;; should return 99
(eval (defun getxx (x) (getx)))
(getxx 2) ;; should also return 99
'done
