(displayln
 (+ 1
    (call/cc
      (lambda (fst)
              (+ 5
                 (call/cc
                   (lambda (snd)
                           (+ 10 (fst 1)))))))))
;
; (define (negative? x) (< x 0))
;
; (define (test)
;   (call/cc
;     (lambda (exit)
;       (for-each (lambda (x)
;                   (if (negative? x)
;                       (begin
;                         (displayln "YO!")
;                         (exit x))))
;                 '(54 0 37 -3 245 19))
;     #t)))
;
; (displayln (test))
