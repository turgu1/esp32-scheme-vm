;; to avoid being constant-folded away
(define zero 0)
(define min-one -1)
(define five 5)
(define two 2)
(define min-five -5)
(define min-two -2)

(displayln (not (< zero zero)))
(displayln (< min-one zero))
(displayln (= five five))
(displayln (< two five))
(displayln (> five two))
(displayln (= min-five min-five))
(displayln (> min-two min-five))
(displayln (< min-five min-two))
(displayln (< min-five 65533))
(displayln (< min-five two))
(displayln (> five -65533))
(displayln (> five min-two))
(displayln (< min-five 65700))