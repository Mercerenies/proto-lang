;; Mode for editing files in the Latitude language

(require 'smie)

(defvar latitude-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "<backtab>") 'latitude-mode-dedent)
    map))

; TODO Lisp mode seems to define a lot of symbols as whitespace; change them to symbols here
(defvar latitude-mode-syntax-table
   (let ((st (make-syntax-table lisp-mode-syntax-table)))
     (modify-syntax-entry ?\; "<" st)
     (modify-syntax-entry ?\n ">" st)
     (modify-syntax-entry ?\{ "(}1n" st)
     (modify-syntax-entry ?\* ". 23n" st)
     (modify-syntax-entry ?\} "){4n" st)
     (modify-syntax-entry ?\[ "(]" st)
     (modify-syntax-entry ?\] ")[" st)
     (modify-syntax-entry ?\. "." st)
     (modify-syntax-entry ?\, "." st)
     (modify-syntax-entry ?\: "." st)
     (modify-syntax-entry ?\= "_" st)
     (modify-syntax-entry ?\~ "_" st)
     (modify-syntax-entry ?\? "_" st)
     (modify-syntax-entry ?\! "_" st)
     (modify-syntax-entry ?\< "_" st)
     (modify-syntax-entry ?\> "_" st)
     (modify-syntax-entry ?\| "_" st)
     (modify-syntax-entry ?\$ "w" st)
     (modify-syntax-entry ?\& "w" st)
     st))

(defvar latitude-mode-font-lock-keywords
  (list `("\\_<\\(\\(?:\\sw\\|\\s_\\)+\\)\\s-*::?=\\s-*{"
          (1 font-lock-function-name-face))
        `("\\_<\\(&?[A-Z]\\(?:\\sw\\|\\s_\\)*\\)\\_>"
          (1 font-lock-type-face))
        `("\\_<\\(\\(?:\\sw\\|\\s_\\)+\\)\\s-*:="
          (1 font-lock-variable-name-face))
        `(,(regexp-opt '("clone" "toString" "pretty" "meta" "global" "lexical" "parent"
                         "here" "again" "self" "invoke" "get" "has?" "put" "slot" "hold" "callCC"
                         "call" "if" "while" "ifTrue" "ifFalse" "not" "or" "and" "loop" "throw"
                         "catch" "handle" "load" "eval" "rethrow" "inject" "iterator"
                         "$dynamic" "scope" "$scope" "cons" "car" "cdr" "proc" "id" "memo" "inject"
                         "member?" "takes" "localize" "this" "brackets" "origin" "resolve"
                         "protect" "thunk" "sigil" "do" "err" "then" "else" "when" "slot?"
                         "caller")
                       'symbols)
          . font-lock-builtin-face)
        `(,(regexp-opt '("Object" "True" "False" "Nil" "Symbol" "String" "Number" "Boolean" "Method"
                         "Proc" "Stream" "Cont" "Exception" "SystemError" "Array" "Kernel"
                         "Sequence" "ArgList" "Collection" "SystemArgError"
                         "SystemCallError" "TypeError" "SlotError" "ContError" "ParseError"
                         "BoundsError" "Mixin" "Cons" "Cell" "Lockbox" "Latchkey" "Cached"
                         "IOError" "Wildcard" "Ellipsis" "Match" "NoMatch" "LazySequence" "Process"
                         "StackFrame" "NotSupportedError" "REPL" "FilePath" "FileHeader")
                       'symbols)
          . font-lock-constant-face)
        `(,(regexp-opt '(":=" "::=" "=" "<-"))
          . font-lock-keyword-face)))

(defun latitude-mode-skip-blanks ()
  (forward-line -1)
  (while (and (not (bobp)) (looking-at-p "\n"))
    (forward-line -1)))

(defun latitude-mode-indent ()
  (interactive)
  (save-excursion
    (beginning-of-line)
    (if (bobp)
        (indent-line-to 0)
      (let ((prev-indent (save-excursion
                           (latitude-mode-skip-blanks)
                           (current-indentation)))
            (curr-indent (current-indentation)))
        (if (<= (+ prev-indent latitude-mode-indent) curr-indent)
            (indent-line-to 0)
          (indent-line-to (+ curr-indent latitude-mode-indent))))))
  (let ((final-indent (current-indentation)))
    (format "~S ~S" (current-column) final-indent)
    (if (< (current-column) final-indent)
        (move-to-column final-indent))))

(defun latitude-mode-dedent ()
  (interactive)
  (save-excursion
    (beginning-of-line)
    (if (bobp)
        (indent-line-to 0)
      (let ((prev-indent (save-excursion
                           (latitude-mode-skip-blanks)
                           (current-indentation)))
            (curr-indent (current-indentation)))
        (if (> latitude-mode-indent curr-indent)
            (indent-line-to (+ prev-indent latitude-mode-indent))
          (indent-line-to (- curr-indent latitude-mode-indent))))))
  (let ((final-indent (current-indentation)))
    (format "~S ~S" (current-column) final-indent)
    (if (< (current-column) final-indent)
        (move-to-column final-indent))))

(add-to-list 'auto-mode-alist '("\\.lat[s]?\\'" . latitude-mode))

(defcustom latitude-mode-indent 2
  "The number of spaces to indent one level of a code block in Latitude."
  :type 'integer
  :safe #'integerp
  :group 'latitude)

(defun latitude-mode ()
  (interactive)
  (kill-all-local-variables)
  (set (make-local-variable 'comment-start) "; ")
  (set (make-local-variable 'comment-end) "")
  (set-syntax-table latitude-mode-syntax-table)
  (use-local-map latitude-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(latitude-mode-font-lock-keywords))
  (set (make-local-variable 'indent-line-function) #'latitude-mode-indent)
  ;;
  (setq major-mode 'latitude-mode)
  (setq mode-name "Latitude")
  ;;
  (run-hooks 'latitude-mode-hook))

(provide 'latitude-mode)
