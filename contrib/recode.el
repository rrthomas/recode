;;; recode.el --- Outils divers autour des jeux de caractères nationaux.
;;; Copyright © 1995-1997 Progiciels Bourbeau-Pinard inc.
;;; François Pinard <pinard@iro.umontreal.ca>, Mars 1995.

;; Ce programme est un logiciel libre; vous pouvez le redistribuer ou le
;; modifier selon les termes de la License Publique Générale de GNU, publiée
;; par la Free Software Foundation (soit la version 3 ou soit, à votre
;; discrétion, toute version ultérieure).

;; Ce programme est distribué pour être utile, mais SANS AUCUNE GARANTIE;
;; sans même la garantie implicite qu'il est DE QUALITÉ LOYALE ET MARCHANDE
;; ou APPROPRIÉ POUR UN BUT PARTICULIER.  Voir la Licence Publique Générale
;; de GNU pour plus de détails.

;; Vous devriez avoir reçu copie de la Licence Publique Générale de GNU
;; avec ce programme; sinon, voir <https://www.gnu.org/licenses/>.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Les outils sont regroupés par thème fonctionnel, un par page.  ;;;
;;; La documentation est au début de chacune de ces pages.	   ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Ces outils ne fonctionnent pleinement qu'avec Emacs 19.  Dans la
;;; mesure où ces corrections ne brisent rien qui fonctionne déjà,
;;; j'accepterai avec plaisir vos corrections pour les autres Emacs,
;;; mais je ne prendrai pas le temps de les développer moi-même.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Installation rapide.  ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Voici une recette d'installation rapide, à l'intention d'usagers
;;; placés dans un environnement où les administrateurs des systèmes
;;; ont une attitude amicale.  Elle présume qu'ils ont déjà installé
;;; Emacs 19, metamail, procmail et recode dans le système, et qu'ils
;;; ont aussi placé charset.el, mime.el, rmailmime.el et metamail.el
;;; dans les répertoires déjà fouillés par Emacs pour son code LISP.
;;; Cela suffira probablement aux débrouillards.  Je vous invite
;;; néanmoins à lire la documentation détaillée qui apparaît, dans les
;;; pages subséquentes de ce fichier.

;;; Voici donc, succintement, une liste des ajouts que vous avez à
;;; faire à quelques fichiers pour activer le tout au mieux (à mon
;;; avis).  Il vous faut éliminer le «;;; » au début de chaque ligne.

;;; Dans ~/.procmailrc:

;;; ======================================================================>
;;; :0 fbw
;;; * ^Content-Transfer-Encoding:.*quoted-printable
;;; | recode /qp..
;;;   :0 Afhw
;;;   | formail -i 'Content-Transfer-Encoding: 8bit'
;;; ======================================================================<

;;; Dans ~/.forward (remplacer "pinard" par votre login):

;;; ======================================================================>
;;; "|IFS=' ' && exec /usr/local/bin/procmail -f- || exit 75 #pinard"
;;; ======================================================================<

;;; Dans ~/.emacs:

;;; ======================================================================>
;;; (setq extended-charset t)
;;; (require 'charset)

;;; (standard-display-european 1)
;;; (require 'iso-syntax))

;;; (global-set-key "\C-cB" 'recode-base64)
;;; (global-set-key "\C-cI" 'recode-ibmpc)
;;; ;; (global-set-key "\C-cL" 'recode-latin1)
;;; (global-set-key "\C-cM" 'recode-macintosh)
;;; (global-set-key "\C-cQ" 'recode-quoted-printable)
;;; (global-set-key "\C-cT" 'recode-texte)

;;; (global-set-key "\C-cE" 'toggle-extended-charset)
;;; (global-set-key "\C-cK" 'toggle-sentence-spacing)
;;; (global-set-key "\C-cL" 'toggle-sorting-charset)
;;; (global-set-key "\C-cR" 'toggle-case-replace)
;;; (global-set-key "\C-cS" 'toggle-case-fold-search)
;;; ======================================================================<

;;; Assurez-vous que ~/.forward et ~/.procmailrc sont lisibles par tout le
;;; monde, avec la commande:

;;; 	chmod 644 ~/.forward ~/.procmailrc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Préparation générale.  ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Pour préparer Emacs à travailler en français avec vous et avec
;;; l'aide des outils documentés ici, il faut copier ce fichier
;;; «recode.el» dans un répertoire où vous placez vos fichiers «.el».
;;; Si vous n'avez pas un tel répertoire, fabriquez-vous en un.  Pour
;;; un, j'utilise ~pinard/share/site-lisp/ à cet effet.  Faites le
;;; remplacement qui s'impose dans l'exemple qui suit.  Ajouter les
;;; lignes suivantes dans votre fichier .emacs (sans les «;;;»), ou si
;;; vous n'en avez pas, créez-le au besoin:

;;;    (setq load-path
;;;          (cons (expand-file-name "~pinard/share/site-lisp")
;;;                load-path))

;;;    (setq extended-charset t)

;;;    (standard-display-european 1)
;;;    (require 'iso-syntax)

;;; Vous n'avez pas à ajouter les lignes suivantes:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Décodages et réencodages de régions.  ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Fonctionne avec Emacs 18, Emacs 19, et probablement XEmacs.

;;; Les outils suivants exigent la préinstallation de recode, qui peut être
;;; obtenu de gnu.org, dans le répertoire pub/gnu, sous le nom
;;; recode-3.4.tar.gz ou mieux, de ftp.iro.umontreal.ca, dans le répertoire
;;; pub/contrib/pinard/gnu, sous le nom recode-3.4d.tar.gz.

;;; Pour décoder une région Emacs vers le Latin-1, il faut utiliser
;;; l'une des commandes suivantes (attention aux majuscules):

;;;    C-c B   si le texte est codé en Base64 de MIME
;;;    C-c I   si le texte est codé en code d'IBM-PC
;;;    C-c L   si le texte est codé en Latin-1 (avec demacs sur MSDOS)
;;;    C-c M   si le texte est codé en code de Macintosh
;;;    C-c Q   si le texte est codé en quoted-printable
;;;    C-c T   si le texte est codé en texte (ou easy-french)

;;; Pour encoder une région de Latin-1 vers l'un des codes indiqués,
;;; il suffit de préfixer la commande par «C-u», ce qui a pour effet
;;; de provoquer le codage contraire, si l'on peut dire.  Undo («C-x u»
;;; ou «C-_») permet de reculer sur un mauvais choix de décodage.

;;; Pour décoder un message lu par RMAIL, il faut utiliser les
;;; commandes d'édition de message du format RMAIL: d'abord «w»
;;; pour éditer, «C-x h» pour placer la région autour du message
;;; entier, puis la commande de décodage, et «C-c C-c» pour finir.
;;; Par exemple, pour décoder un message en quoted-printable, faire:

;;;    w C-x h C-c Q C-c C-c

;;; Ces techniques n'ajustent pas les en-têtes MIME du message.

;;; Pour toutes ces fonctions, dans votre .emacs, placez:

;;;    (global-set-key "\C-cB" 'recode-base64)
;;;    (global-set-key "\C-cI" 'recode-ibmpc)
;;;    ;; (global-set-key "\C-cL" 'recode-latin1)
;;;    (global-set-key "\C-cM" 'recode-macintosh)
;;;    (global-set-key "\C-cQ" 'recode-quoted-printable)
;;;    (global-set-key "\C-cT" 'recode-texte)

;;;    (autoload 'recode-base64 "recode" nil t)
;;;    (autoload 'recode-ibmpc "recode" nil t)
;;;    (autoload 'recode-latin1 "recode" nil t)
;;;    (autoload 'recode-macintosh "recode" nil t)
;;;    (autoload 'recode-quoted-printable "recode" nil t)
;;;    (autoload 'recode-texte "recode" nil t)

;;; Vous n'avez pas à ajouter les lignes suivantes:

(defun recode-base64 (flag)
  "Décoder la région courante qui se trouve déjà en base64 de MIME.  Si
cette commande est préfixée (par «C-u»), encoder la région courante en
base64 de MIME."
  (interactive "P")
  (shell-command-on-region (region-beginning) (region-end)
			   (concat "recode " (if flag "../64" "/64..")) t)
  (exchange-point-and-mark))

(defun recode-ibmpc (flag)
  "Décoder la région courante qui se trouve déjà exprimée avec le jeu
de caractères de l'IBM-PC.  Si cette commande est préfixée (par «C-u»),
encoder la région courante dans le jeu de caractères de l'IBM-PC."
  (interactive "P")
  (shell-command-on-region (region-beginning) (region-end)
			   (concat "recode " (if flag "..437" "437..")) t)
  (exchange-point-and-mark))

(defun recode-latin1 (flag)
  "Décoder la région courante qui se trouve déjà exprimée avec le jeu
de caractères ISO 8859-1 (ou Latin-1).  Si cette commande est préfixée
(par «C-u»), encoder la région courante dans le jeu de caractères ISO
8859-1 (ou Latin-1)."
  (interactive "P")
  (shell-command-on-region (region-beginning) (region-end)
			   (concat "recode " (if flag "..l1" "l1..")) t)
  (exchange-point-and-mark))

(defun recode-macintosh (flag)
  "Décoder la région courante qui se trouve déjà exprimée avec le jeu
de caractères du Macintosh.  Si cette commande est préfixée (par «C-u»),
encoder la région courante dans le jeu de caractères du Macintosh."
  (interactive "P")
  (shell-command-on-region (region-beginning) (region-end)
			   (concat "recode " (if flag "..mac" "mac..")) t)
  (exchange-point-and-mark))

(defun recode-quoted-printable (flag)
  "Décoder la région courante qui se trouve déjà en quoted-printable
de MIME.  Si cette commande est préfixée (par «C-u»), encoder la région
courante en quoted-printable de MIME."
  (interactive "P")
  (shell-command-on-region (region-beginning) (region-end)
			   (concat "recode " (if flag "../qp" "/qp..")) t)
  (exchange-point-and-mark))

(defun recode-texte (flag)
  "Décoder la région courante qui se trouve déjà exprimée avec la
convention Easy French, où l'accent est placée après la lettre plutôt
qu'au-dessus.  Si cette commande est préfixée (par «C-u»), encoder la
région courante avec la convention Easy French."
  (interactive "P")
  (shell-command-on-region (region-beginning) (region-end)
			   (concat "recode " (if flag "..txte" "txte..")) t)
  (exchange-point-and-mark))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Affichage, fouilles, remplacements et tris français.  ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Fonctionne généralement avec Emacs 18 sauf pour «C-c K»; «C-c E»
;;; ne fonctionne que des versions spécialement modifiées par moi,
;;; versions plutôt rares de nos jours.  Fonctionne avec Emacs 19.
;;; J'ignore pour XEmacs.

;;; Ces outils contrôlent l'effet de la capitalisation et des diacritiques
;;; sur l'affichage, sur les fouilles avec ou sans remplacement, et
;;; sur les tris dans Emacs.  Notez les majuscules dans les clefs de
;;; commande décrites ci-après.

;;; La commande «C-c E» est une bascule entre l'affichage français ou
;;; l'affichage avec des séquences d'échappement en octal, dans les
;;; fenêtres de Emacs.

;;; La commande «C-c S» est une bascule entre le fait que les divers
;;; outils de fouilles de Emacs ignoreront, ou non, la distinction
;;; entre majuscules et minuscules.  Les diacritiques ne sont pas ignorés.

;;; La commande «C-c R» est une bascule entre le fait que les
;;; commandes de fouille et remplacement de Emacs essaient, ou non, de
;;; rétablir dans la chaîne remplacée la capitalisation de la chaîne
;;; appariant le gabarit de fouille.  Les diacritiques ne sont pas ignorés.

;;; La commande «C-c K» est une bascule entre le fait que les
;;; commandes de tri de Emacs utiliseront, ou non, l'ordre
;;; lexicographique français.  Dans les comparaisons françaises, les
;;; diacritiques ne sont pris en considération que si les chaînes comparées
;;; sont par ailleurs identiques.  Les commandes Emacs affectées par
;;; cette bascule sont:

;;;     Fonction Emacs        Unité de tri     Clef

;;;     sort-lines            lignes           toute la ligne
;;;     sort-columns          lignes           le contenu entre deux colonnes
;;;     sort-fields           lignes           le champ indiqué en argument
;;;     sort-pages            pages            toute la page
;;;     sort-paragraphs       paragraphes      tout le paragraphe
;;;     sort-regexp-fields    occurrences      toute l'occurrence ou \N

;;; Pour toutes ces fonctions, dans votre .emacs, placez:

;;;    (global-set-key "\C-cE" 'toggle-extended-charset)
;;;    (global-set-key "\C-cK" 'toggle-sentence-spacing)
;;;    (global-set-key "\C-cL" 'toggle-sorting-charset)
;;;    (global-set-key "\C-cR" 'toggle-case-replace)
;;;    (global-set-key "\C-cS" 'toggle-case-fold-search)

;;;    (autoload 'toggle-extended-charset "charset" nil t)
;;;    (autoload 'toggle-sentence-spacing "charset" nil t)
;;;    (autoload 'toggle-sorting-charset "charset" nil t)
;;;    (autoload 'toggle-case-replace "charset" nil t)
;;;    (autoload 'toggle-case-fold-search "charset" nil t)

;;; Vous n'avez pas à ajouter les lignes suivantes:

(defun toggle-variable (variable message-on message-off)
  (if (eval variable)
      (progn
	(set variable nil)
	(message message-off))
    (set variable t)
    (message message-on)))

(defun toggle-extended-charset ()
  "Basculer entre le mode d'affichage 8bit pour l'ISO 8859-1 (ou
Latin-1) et le mode d'affichage 7bit, où les caractères dépassant 127
sont exprimés par un «\\» suivi de la valeur octale du caractère."
  (interactive)
  (toggle-variable 'extended-charset
		   "Mode d'affichage 8bit pour l'ISO Latin-1"
		   "Mode d'affichage 7bit pour l'ASCII")
  (standard-display-european (if extended-charset 1 -1))
  (redraw-display))

;; N'est utile que sur des versions récentes de Emacs.  1995-05-04.
;;
(defun toggle-sentence-spacing ()
  "Basculer entre deux espaces requis, ou un espace suffisant, à la fin d'une
phrase, pour les commandes de GNU Emacs opérant sur des phrases."
  (interactive)
  (toggle-variable 'sentence-end-double-space
		   "Plus d'un espace requis pour terminer une phrase"
		   "Une fin de phrase peut n'être suivie que d'un espace"))

(defun toggle-sorting-charset ()
  "Basculer l'ordre lexicographique utilisé pour les tris entre la
convention utilisée pour ordonner des mots français, et la convention
se basant uniquement sur la valeur numérique binaire des caractères."
  (interactive)
  (toggle-variable 'sorting-charset
		   "Tris en ordre lexicographique français"
		   "Tris en ordre lexicographique binaire")
  (charset-sorts sorting-charset))

(defun toggle-case-fold-search ()
  "Basculer entre le repli des majuscules/minuscules durant les
fouilles, et l'absence de tels replis."
  (interactive)
  (toggle-variable 'case-fold-search
		   "Fouilles avec repli majuscules/minuscules"
		   "Fouilles sans repli majuscules/minuscules"))

(defun toggle-case-replace ()
  "Basculer entre le fait que les remplacements cherchent, ou non, à
réajuster les majuscules/minuscules selon la capitalisation de chaque
chaîne à remplacer."
  (interactive)
  (toggle-variable 'case-replace
		   "Remplacements avec majuscules/minuscules réajustées"
		   "Remplacements sans ajustement de majuscule/minuscule"))

(setq charset-to-base
      (concat
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\0ABCDEFGHIJKLMNOPQRSTUVWXYZ\0\0\0\0\0"
       "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "AAAAAAACEEEEIIII\0NOOOOO\0OUUUUY\0\0"
       "aaaaaaaceeeeiiii\0nooooo\0ouuuuy\0y"))

(setq charset-to-diac
      (concat
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\4\0"
       "\3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\6\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
       "\3\2\4\6\5\8\1\7\3\2\4\5\3\2\4\5\0\6\3\2\4\6\5\0\9\3\2\4\5\2\0\0"
       "\3\2\4\6\5\8\1\7\3\2\4\5\3\2\4\5\0\6\3\2\4\6\5\0\9\3\2\4\5\2\0\0"))

(let ((counter 0))
  (while (< counter 256)
    (if (= (aref charset-to-base counter) 0)
	(aset charset-to-base counter counter))
    (setq counter (1+ counter))))

(defvar sorting-charset nil)

(or (fboundp 'builtin-compare-buffer-substrings)
    (fset 'builtin-compare-buffer-substrings
	  (symbol-function 'compare-buffer-substrings)))

(defun charset-compare-buffer-substrings
  (left-buffer left-start left-end right-buffer right-start right-end)

  (or left-buffer (setq left-buffer (current-buffer)))
  (or right-buffer (setq right-buffer (current-buffer)))

  (let (lstart lend rstart rend result)
    (save-excursion

      ;; Préparer le tampon de gauche avec les bases.

      (set-buffer charset-sorts-left-buffer)
      (erase-buffer)
      (insert-buffer-substring left-buffer left-start left-end)
      (setq lstart (point-min))
      (setq lend (point-max))
      (translate-region lstart lend charset-to-base)
      (and case-fold-search (upcase-region lstart lend))

      ;; Préparer le tampon de droite avec les bases.

      (set-buffer charset-sorts-right-buffer)
      (erase-buffer)
      (insert-buffer-substring right-buffer right-start right-end)
      (setq rstart (point-min))
      (setq rend (point-max))
      (translate-region rstart rend charset-to-base)
      (and case-fold-search (upcase-region rstart rend))

      ;; Comparer les bases.

      (setq result (builtin-compare-buffer-substrings
		    charset-sorts-left-buffer lstart lend
		    charset-sorts-right-buffer rstart rend))
      (if (= result 0)
	  (progn

	    ;; Préparer le tampon de gauche avec les diacritiques.

	    (set-buffer charset-sorts-left-buffer)
	    (erase-buffer)
	    (insert-buffer-substring left-buffer left-start left-end)
	    (setq lstart (point-min))
	    (setq lend (point-max))
	    (translate-region lstart lend charset-to-diac)

	    ;; Préparer le tampon de droite avec les diacritiques.

	    (set-buffer charset-sorts-right-buffer)
	    (erase-buffer)
	    (insert-buffer-substring right-buffer right-start right-end)
	    (setq rstart (point-min))
	    (setq rend (point-max))
	    (translate-region rstart rend charset-to-diac)

	    ;; Comparer les diacritiques.

	    (setq result (builtin-compare-buffer-substrings
			  charset-sorts-left-buffer lstart lend
			  charset-sorts-right-buffer rstart rend)))))

    ;; Retourner le résultat de la comparaison.

    result))

;;; Faire en sorte que les tris utilisent l'ordre lexicographique
;;; français sous-jacent à l'ISO Latin-1.  Ne comparer les
;;; diacritiques qu'en dernier ressort, lorsque la comparaison donne
;;; égal par ailleurs.  Un argument nil rétablit le traitement
;;; implicite du départ.

(defun charset-sorts (latin1)
  (if latin1
      (progn
	(fset 'compare-buffer-substrings 'charset-compare-buffer-substrings)
	(setq charset-sorts-left-buffer
	      (get-buffer-create "*Latin1 sorts left*"))
	(setq charset-sorts-right-buffer
	      (get-buffer-create "*Latin1 sorts right*")))
    (fset 'compare-buffer-substrings 'builtin-compare-buffer-substrings)
    (kill-buffer charset-sorts-left-buffer)
    (kill-buffer charset-sorts-right-buffer)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Interfaces pour les codages MIME.  ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Fonctionne avec Emacs 19.  J'ignore pour les autres.

;;; Trois produits différents vous faciliteront l'usage de MIME dans
;;; Emacs: ce sont metamail, procmail et emacs-mime.  Nous les
;;; discutons séparément.

;;; -------- ;;;
;;; metamail ;;;
;;; -------- ;;;

;;; Le metamail de Nathaniel Boreinstein est absolument requis, pour
;;; fournir, entre autres choses, les convertions de format de base.
;;; Nous en avons déjà discuté, ailleurs dans ce fichier.  metamail
;;; est disponible au site thumper.bellcore.com, dans le répertoire
;;; pub/nsb, sous le nom mm2.7.tar.Z.  Il suffit de l'installer pour
;;; que les binaires soient accessibles de votre PATH.

;;; -------- ;;;
;;; procmail ;;;
;;; -------- ;;;

;;; Le procmail de Stephen R. van den Berg, n'est pas requis, mais
;;; c'est un outil luxueux que je vous recommande beaucoup.  Il
;;; convertira automatiquement les messages MIME en quoted-printable,
;;; format simple mais habituel, directement en Latin-1, avant même
;;; que vous récupériez vos messages.  procmail peut être obtenu de
;;; ftp.informatik.rwth-aachen.de, dans pub/packages/procmail, sous le
;;; nom procmail.tar.gz.  Il vous suffit de l'installer quelque part.

;;; Pour activer procmail, vous n'avez rien à faire dans Emacs à
;;; proprement parler.  Fabriquez-vous un fichier ~/.forward, lisible
;;; par tout le monde, et contenant (y compris les guillemets):

;;;    "|IFS=' ' && exec /usr/local/bin/procmail -f- || exit 75 #pinard"

;;; en remplacant «pinard» par votre nom login et «/usr/local/bin» par
;;; le répertoire contenant le binaire exécutable de procmail.
;;; Fabriquez-vous ensuite un fichier ~/.procmailrc contenant:

;;;    :0 fbw
;;;    * ^Content-Transfer-Encoding:.*quoted-printable
;;;    | recode /qp..
;;;      :0 Afhw
;;;      | formail -i 'Content-Transfer-Encoding: 8bit'

;;; Il est possible qu'il faille remplacer les appels à «recode» et
;;; «formail» (distribué dans procmail) en y adjoignant le *path* complet,
;;; puisque ces programmes seront exécutés par le système en dehors de votre
;;; environnement habituel.

;;; Compatibilité avec versions précédentes de ce fichier.

(fset 'toggle-sorting-latin1 (symbol-function 'toggle-sorting-charset))

;;; Annoncer que le chargement est complet.

(provide 'recode)
