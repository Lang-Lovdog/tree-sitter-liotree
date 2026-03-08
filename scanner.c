#include <tree_sitter/parser.h>
#include <stdlib.h>
#include <string.h>

enum TokenType { INDENT, DEDENT, NEWLINE };

#define MAX_DEPTH 100

typedef struct {
  uint16_t stack[MAX_DEPTH];
  uint16_t len;
} Scanner;

void *tree_sitter_liotree_external_scanner_create() {
  Scanner *s = malloc(sizeof(Scanner));
  memset(s->stack, 0, sizeof(s->stack));
  s->len = 1; // Base depth is 0
  return s;
}

void tree_sitter_liotree_external_scanner_destroy(void *payload) {
  free(payload);
}

unsigned tree_sitter_liotree_external_scanner_serialize(void *payload, char *buffer) {
  Scanner *s = (Scanner *)payload;
  uint16_t size = s->len * sizeof(uint16_t);
  memcpy(buffer, s->stack, size);
  return size;
}

void tree_sitter_liotree_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
  Scanner *s = (Scanner *)payload;
  s->len = 0;
  if (length > 0) {
    s->len = length / sizeof(uint16_t);
    memcpy(s->stack, buffer, length);
  } else {
    s->stack[0] = 0;
    s->len = 1;
  }
}

bool tree_sitter_liotree_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
  Scanner *s = (Scanner *)payload;

  // 1. Always prioritize NEWLINE to end the current entry node
  if (valid_symbols[NEWLINE] && (lexer->lookahead == '\n' || lexer->lookahead == '\r')) {
    lexer->advance(lexer, false);
    lexer->result_symbol = NEWLINE;
    return true;
  }

  // 2. Handle INDENT/DEDENT via lookahead
  if (valid_symbols[INDENT] || valid_symbols[DEDENT]) {
    
    // Skip spaces to find the start of the next line logic
    while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
        lexer->advance(lexer, true);
    }

    if (lexer->lookahead == '|') {
      // We found a bar! Now we count dashes without "consuming" them
      // In Tree-sitter, we can't truly 'peek' far, so we 'advance'
      // but we tell the lexer this is NOT the end of the token.
      
      uint16_t count = 0;
      // We temporarily advance to count, but we won't call lexer->mark_end()
      // so the main grammar can re-read these characters.
      lexer->advance(lexer, false); // skip '|'
      
      while (lexer->lookahead == '-' || lexer->lookahead == '+') {
        count++;
        lexer->advance(lexer, false);
      }

      uint16_t current_depth = s->stack[s->len - 1];

      if (count > current_depth && valid_symbols[INDENT]) {
        // Push to stack and emit INDENT
        s->stack[s->len++] = count;
        lexer->result_symbol = INDENT;
        // Do NOT call mark_end here. Let the grammar start at the '|'
        return true; 
      }

      if (count < current_depth && valid_symbols[DEDENT]) {
        // Pop and emit DEDENT
        s->len--;
        lexer->result_symbol = DEDENT;
        return true;
      }
    }
  }

  return false;
}
