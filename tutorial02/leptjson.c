#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
	const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	c->json = p;
}


static int lept_parse_literal(lept_context* c, lept_value* v) {
	if (c->json[0] == 't' && c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e') {
		c->json += 4;
		v->type = LEPT_TRUE;
		return LEPT_PARSE_OK;
	}
	else if (c->json[0] == 'f' && c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e') {
		c->json += 5;
		v->type = LEPT_FALSE;
		return LEPT_PARSE_OK;
	}
	else if (c->json[0] == 'n' && c->json[1] == 'u' && c->json[2] == 'l' && c->json[3] == 'l') {
		c->json += 4;
		v->type = LEPT_NULL;
		return LEPT_PARSE_OK;
	}
	else {
		return LEPT_PARSE_INVALID_VALUE;
	}
}


static int verify_number(char* s) {
	char* p = s;
	bool exponent_signed = false;
	//step 1 = validate number, step 2 = validating fraction, step 3 = verifying exponent
	int step = 1;
	//check if first character is valid
	if (!(ISDIGIT(*p) || *p == '-')) {
		return LEPT_PARSE_INVALID_VALUE;
	}
	if (*p == '-') {
		p++;
	}

	//check the second character
	if (*p == '0') {
		p++;
		if (*p == 'E' || *p == 'e') {
			step = 3;
		}
		else if (*p == '.') {
			step = 2;
		}
		else if (*p == '\0') {
			return LEPT_PARSE_OK;
		}
		else {
			return LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	p++;
	while (*p != '\0') {
		if (step == 1) {
			if (ISDIGIT1TO9(*p) == false) {
				if (*p == '.') {
					step = 2;
				}
				else if (*p == 'e' || *p == 'E') {
					step = 3;
				}
				else {
					return LEPT_PARSE_INVALID_VALUE;
				}
			}
		}
		else if (step == 2) {
			if (ISDIGIT(*p) == false) {
				if (*p == 'e' || *p == 'E') {
					step = 3;
				}
				else {
					return LEPT_PARSE_INVALID_VALUE;
				}
			}
		}
		else if (step == 3) {
			if (*p == '+' || *p == '-') {
				if (exponent_signed == false) {
					exponent_signed = true;
				}
				else {
					return LEPT_PARSE_INVALID_VALUE;
				}
			}
			else if (ISDIGIT(*p) == false) {
				return LEPT_PARSE_INVALID_VALUE;
			}
		}
		p++;
	}
	if (ISDIGIT(*(p - 1))) {
		return LEPT_PARSE_OK;
	}
	else {
		return LEPT_PARSE_INVALID_VALUE;
	}

}

static int lept_parse_number(lept_context* c, lept_value* v) {
	char* end;
	/* \TODO validate number */
	int result = verify_number(c->json);
	if (result != LEPT_PARSE_OK) {
		return result;
	}
	v->n = strtod(c->json,  &end);
	if (c->json == end)
		return LEPT_PARSE_INVALID_VALUE;
	c->json = end;
	v->type = LEPT_NUMBER;
	return LEPT_PARSE_OK;
}


static int lept_parse_value(lept_context* c, lept_value* v) {
	switch (*c->json) {
	case 't':  return lept_parse_literal(c, v);
	case 'f':  return lept_parse_literal(c, v);
	case 'n':  return lept_parse_literal(c, v);
	default:   return lept_parse_number(c, v);
	case '\0': return LEPT_PARSE_EXPECT_VALUE;
	}
}

int lept_parse(lept_value* v, const char* json) {
	lept_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	v->type = LEPT_NULL;
	lept_parse_whitespace(&c);
	if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_whitespace(&c);
		if (*c.json != '\0') {
			v->type = LEPT_NULL;
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	return ret;
}

lept_type lept_get_type(const lept_value* v) {
	assert(v != NULL);
	return v->type;
}

double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->n;
}
