/**
 * @file This is Lang Lovdog's format for directory tree representation
 * @author Lang Lovdog Inu Oókami <hataraku_wulfus@outlook.com>
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

module.exports = grammar({
  name: "liotree",

  // Do not include \n in extras so we can anchor rules to line starts
  extras: $ => [/[\s\t\r]/],

  rules: {
    source_file: $ => $.root,

    _item: $ => choice(
      $.file_entry,
      $.directory_entry,
      $.comment
    ),


    // Fixed Root: Recognizes | followed by a directory name
    root: $ => seq(
      $.root_bar,
      " ",
      field("name", $.directory_name),
      optional($.comment),
      /\n/,
      field("contents", $.content_block)
    ),

    root_bar: $ => "|",

    // Fixed Entry: Splits the bar from the dashes to prevent "conceal eating"
    entry_bar: $ => "|",

    format_space: $ => /\ /,

    depth_mark: $ => /[-+]/,

    scope_delimiter_open: $ => seq(
        "|=-", 
        optional($.comment), 
        /\n/
    ),

    scope_delimiter_closed: $ => seq(
        "|==", 
        optional($.comment), 
        /\n/
    ),

    file_entry: $ => seq(
      $.entry_bar,
      field("bridge", optional(repeat($.depth_mark))),
      field("leaf", $.depth_mark),
      " ",
      field("name", $.file_name),
      optional($.comment),
      /\n/
    ),

    directory_entry: $ => prec.left(seq(
      $.entry_bar,
      field("bridge", optional(repeat($.depth_mark))),
      field("leaf", $.depth_mark),
      " ",
      field("name", $.directory_name),
      optional($.comment),
      /\n/,
      optional(seq(
        $.scope_delimiter_open,
        field("contents", $.content_block),
        $.scope_delimiter_closed,
      ))
    )),

    directory_name: $ => seq(
      /[^-+|>\s][^|>\n]*\/['"]{0,1}/,
      field("formatspace", optional(repeat($.format_space)))
    ),

    file_name: $ => seq(
      choice(
      /[^-+|>\s][^|>\/\n\s]*/,
      /\"[^|>\/\n]*\"/,
      /\'[^|>\/\n]*\'/
      ),
      field("formatspace", optional(repeat($.format_space)))
    ),

    content_block: $ => repeat1($._item),

    comment: $ => seq(
      $.comment_open,
      $.comment_text,
      optional($.comment_content),
      $.comment_closed
    ),

    comment_open: $ => token('>'),

    comment_closed: $ => token('||'),

    comment_text: $ => token(/[^|\n]+/),

    comment_newline: $ => seq(
      repeat(/\n\ */),
      $.comment_bar,
      repeat(/\ */)
    ),

    comment_bar: $ => "|",

    comment_content: $ => seq(
      $.comment_newline,
      repeat(choice(
        prec(2, $.comment_newline), 
        prec(1, $.comment_text)
      ))
    ),
  }
});
