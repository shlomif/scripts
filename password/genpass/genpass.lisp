(defconstant +cclen+ 64)
(defparameter *code2char*
  (make-array +cclen+ :initial-contents
              '(#\a #\b #\c #\d #\e #\f #\g #\h #\i #\j #\k #\l #\m #\n #\o
                #\p #\q #\r #\s #\t #\u #\v #\w #\x #\y #\z #\A #\B #\C #\D
                #\E #\F #\G #\H #\I #\J #\K #\L #\M #\N #\O #\P #\Q #\R #\S
                #\T #\U #\V #\W #\X #\Y #\Z #\0 #\1 #\2 #\3 #\4 #\5 #\6 #\7
                #\8 #\9 #\+ #\/)))
(defun genpass (length)
  (with-open-file (fh "/dev/urandom" :element-type 'unsigned-byte)
    (let ((buf (make-array length)))
      (read-sequence buf fh)
      (map 'string (lambda (c) (aref *code2char* (mod c +cclen+))) buf))))
(genpass 8)
