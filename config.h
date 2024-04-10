/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H

/* indexed by enum expr_type */
const char* const expr_type_str[15] = {
	"PATH",
	"CONTENT",
	"INCL",
	"IF",
	"ENDIF",
	"IFNOT",
	"ENDIFNOT",
	"FOR",
	"ENDFOR",
	"REVFOR",
	"ENDREVFOR",
	"FORALL",
	"ENDFORALL",
	"REVFORALL",
	"ENDREVFORALL"
};

#endif /* CONFIG_H */
