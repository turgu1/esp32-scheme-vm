(define x (make-u8vector 2 56))
(define j 0)

(let loopit ((i 0))
  (if (< i 50000)
      (begin (displayln i)
             (if (not (= (u8vector-ref (u8vector 1 2 3) 2) 3))
                 (displayln "BAD!"))
             (loopit (+ i 1))
        )))
(displayln "DONE")
(displayln (u8vector-ref x 1))
