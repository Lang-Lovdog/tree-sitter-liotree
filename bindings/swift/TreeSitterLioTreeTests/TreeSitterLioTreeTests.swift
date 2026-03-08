import XCTest
import SwiftTreeSitter
import TreeSitterLiotree

final class TreeSitterLiotreeTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_liotree())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Lang Lovdog Tree Format grammar")
    }
}
