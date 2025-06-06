#ifndef QT_NO_DEBUG
#include "latexparser/latexparsing.h"
#include "latexparsing_t.h"

#include "qdocument.h"
#include "qdocumentline.h"
#include "qdocumentline_p.h"

#include "latexparser/latexparser.h"
#include "latexpackage.h"
#include "testutil.h"
#include "configmanager.h"
#include <QtTest/QtTest>

// shortcuts and semantic types
typedef Token T;
typedef QList<Token::TokenType> TTypes;
typedef QList<Token::TokenType> STypes;
typedef QList<int> Length;
typedef QList<int> Starts;
typedef QList<int> Levels;

typedef QList<int> ATypes;

Q_DECLARE_METATYPE(TTypes)

void LatexParsingTest::test_simpleLexing_data() {
    QTest::addColumn<QString>("line");
    QTest::addColumn<TTypes>("types");
    QTest::addColumn<Starts>("starts");
    QTest::addColumn<Length>("lengths");

    QTest::newRow("simple") << "bummerang"
                            << (TTypes() << T::word)
							<< (Starts() << 0)
							<< (Length() << 9);
    QTest::newRow("command") << "\\bummerang"
							 << (TTypes() << T::command)
							 << (Starts() << 0)
							 << (Length() << 10);
    QTest::newRow("command with symbol") << "\\_"
										 << (TTypes() << T::command)
										 << (Starts() << 0)
										 << (Length() << 2);
    QTest::newRow("command with symbol2") << "\\&"
										  << (TTypes() << T::command)
										  << (Starts() << 0)
										  << (Length() << 2);
    QTest::newRow("command with symbol3") << "\\__"
										  << (TTypes() << T::command << T::punctuation)
										  << (Starts() << 0 << 2)
										  << (Length() << 2 << 1);
    QTest::newRow("command with at") << "\\bummer@ng"
									 << (TTypes() << T::command)
									 << (Starts() << 0)
									 << (Length() << 10);
    QTest::newRow("command with arg") << "\\bummerang{abc}"
									  << (TTypes() << T::command << T::openBrace << T::word << T::closeBrace)
									  << (Starts() << 0 << 10 << 11 << 14)
									  << (Length() << 10 << 1 << 3 << 1);
    QTest::newRow("command with optional arg") << " \\bummerang  [abc  ]"
											   << (TTypes() << Token::command << T::openSquare << T::word << T::closeSquareBracket)
											   << (Starts() << 1 << 13 << 14 << 19)
											   << (Length() << 10 << 1 << 3 << 1);
    QTest::newRow("punctation") << "bummerang."
								<< (TTypes() << T::word << T::punctuation)
								<< (Starts() << 0 << 9)
								<< (Length() << 9 << 1);
    QTest::newRow("symbol") << " a = b "
							<< (TTypes() << T::word << T::symbol << T::word)
							<< (Starts() << 1 << 3 << 5)
							<< (Length() << 1 << 1 << 1);
    QTest::newRow("umlaut") << "\"uber"
							<< (TTypes() << T::punctuation << T::word)
							<< (Starts() << 0 << 1)
							<< (Length() << 1 << 4);
    QTest::newRow("numbers") << "1.2"
							 << (TTypes() << T::number << T::punctuation << T::number)
							 << (Starts() << 0 << 1 << 2)
							 << (Length() << 1 << 1 << 1);
    QTest::newRow("numbers1") << "1em"
							  << (TTypes() << T::number << T::word)
							  << (Starts() << 0 << 1)
							  << (Length() << 1 << 2);
    QTest::newRow("numbers2") << "em2a"
							  << (TTypes() << T::word << T::number << T::word)
							  << (Starts() << 0 << 2 << 3)
							  << (Length() << 2 << 1 << 1);
    QTest::newRow("numbers3") << "\\text1"
							  << (TTypes() << T::command << T::number)
							  << (Starts() << 0 << 5)
							  << (Length() << 5 << 1);
    QTest::newRow("numbers4") << "45\\text4em"
							  << (TTypes() << T::number << T::command << T::number << T::word)
							  << (Starts() << 0 << 2 << 7 << 8)
							  << (Length() << 2 << 5 << 1 << 2);
    QTest::newRow("command with brackets") << "\\bummerang (abc,fgew)"
										   << (TTypes() << T::command << T::openBracket << T::word << T::punctuation << T::word << T::closeBracket)
										   << (Starts() << 0 << 11 << 12 << 15 << 16 << 20)
										   << (Length() << 10 << 1 << 3 << 1 << 4 << 1);
    QTest::newRow("comments") << "abc % \\text"
                              << (TTypes() << T::word << T::comment << T::command )
                              << (Starts() << 0 << 4 << 6 )
                              << (Length() << 3 << 1 << 5 );
}

void LatexParsingTest::test_simpleLexing() {
    QFETCH(QString, line);
    QFETCH(TTypes, types);
    QFETCH(Starts, starts);
    QFETCH(Length, lengths);
    QDocument *doc = new QDocument();
    QDocumentLineHandle *dlh = new QDocumentLineHandle(line, doc);
    Parsing::simpleLexLatexLine(dlh);
    TokenList tl = dlh->getCookieLocked(QDocumentLine::LEXER_RAW_COOKIE).value<TokenList>();
    for(int i=0; i<tl.length(); i++) {
        Token tk = tl.at(i);
        QVERIFY2(tk.type==types.value(i), QString("incorrect type at index %1:%2").arg(i).arg(line).toLatin1());
        QVERIFY2(tk.start == starts.value(i), QString("incorrect start at index %1").arg(i).toLatin1());
        QVERIFY2(tk.length == lengths.value(i), QString("incorrect length at index %1").arg(i).toLatin1());
    }
    dlh->deref();
    delete doc;
}

void LatexParsingTest::test_latexLexing_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<TTypes>("types");
    QTest::addColumn<STypes>("subtypes");
    QTest::addColumn<Starts>("starts");
    QTest::addColumn<Length>("lengths");
    QTest::addColumn<Levels>("levels");

    QTest::newRow("simple") << "bummerang"
                            << (TTypes() << T::word)
                            << (STypes() << T::none)
                            << (Starts() << 0)
                            << (Length() << 9)
                            << (Levels() << 0);
    QTest::newRow("simple2") << "bummerang test"
                             << (TTypes() << T::word << T::word)
                             << (STypes() << T::none << T::none)
                             << (Starts() << 0 << 10)
                             << (Length() << 9 << 4)
                             << (Levels() << 0 << 0);
    QTest::newRow("simple3") << "bummerang\ntest"
                             << (TTypes() << T::word << T::word)
                             << (STypes() << T::none << T::none)
                             << (Starts() << 0 << 0)
                             << (Length() << 9 << 4)
                             << (Levels() << 0 << 0);
    QTest::newRow("text command") << "\\textbf{text}"
                                  << (TTypes() << T::command << T::braces << T::word)
                                  << (STypes() << T::none << T::text << T::text)
                                  << (Starts() << 0 << 7 << 8)
                                  << (Length() << 7 << 6 << 4)
                                  << (Levels() << 0 << 1 << 1);
    QTest::newRow("text command with nested braces") << "\\textbf{text {abc}}"
                                  << (TTypes() << T::command << T::braces << T::word << T::braces << T::word)
                                  << (STypes() << T::none << T::text << T::text << T::none << T::none)
                                  << (Starts() << 0 << 7 << 8 << 13 << 14)
                                  << (Length() << 7 << 12 << 4 << 5 << 3)
                                  << (Levels() << 0 << 1 << 1 << 1 << 1);
    QTest::newRow("text command with nested square brackets") << "\\textbf{text [abc]}"
                                  << (TTypes() << T::command << T::braces << T::word << T::word)
                                  << (STypes() << T::none << T::text << T::text << T::text)
                                  << (Starts() << 0 << 7 << 8 << 14)
                                  << (Length() << 7 << 12 << 4 << 3)
                                  << (Levels() << 0 << 1 << 1 << 1 );
    QTest::newRow("section command") << "\\section{text}"
                                     << (TTypes() << T::command << T::braces << T::word)
                                     << (STypes() << T::none << T::title << T::title)
                                     << (Starts() << 0 << 8 << 9)
                                     << (Length() << 8 << 6 << 4)
                                     << (Levels() << 0 << 1 << 1);
    QTest::newRow("section command, multi-line") << "\\section{text\ntest}"
                                     << (TTypes() << T::command << T::openBrace << T::word<< T::word<<T::closeBrace)
                                     << (STypes() << T::none << T::title << T::title<<T::title<<T::title)
                                     << (Starts() << 0 << 8 << 9<< 0 << 4)
                                     << (Length() << 8 << 5 << 4<< 4 << 1)
                                     << (Levels() << 0 << 1 << 1<< 1 << 1);
    QTest::newRow("section command, multi-line optional") << "\\section[ab\ncd]{text\ntest}"
                                     << (TTypes() << T::command << T::openSquare << T::word<< T::word<<T::closeSquareBracket<< T::openBrace << T::word<< T::word<<T::closeBrace)
                                     << (STypes() << T::none <<T::shorttitle << T::shorttitle<<T::shorttitle<<T::shorttitle << T::title << T::title<<T::title<<T::title)
                                     << (Starts() << 0 << 8 << 9 << 0 << 2 << 3 << 4 << 0 << 4)
                                     << (Length() << 8 << 3 << 2 << 2 << 1 << 5 << 4 << 4 << 1)
                                     << (Levels() << 0 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1);
    QTest::newRow("usepackage command") << "\\usepackage{text}"
                                        << (TTypes() << T::command << T::braces << T::package)
                                        << (STypes() << T::none << T::package << T::none)
                                        << (Starts() << 0 << 11 << 12)
                                        << (Length() << 11 << 6 << 4)
                                        << (Levels() << 0 << 1 << 1);
    QTest::newRow("usepackage command2") << "\\usepackage{text,text}"
                                         << (TTypes() << T::command << T::braces << T::package << T::package)
                                         << (STypes() << T::none << T::package << T::none << T::none)
                                         << (Starts() << 0 << 11 << 12 << 17)
                                         << (Length() << 11 << 11 << 4 << 4)
                                         << (Levels() << 0 << 1 << 1 << 1);
    QTest::newRow("usepackage command3") << "\\usepackage{text,\ntext}"
                                         << (TTypes() << T::command << T::openBrace << T::package << T::package << T::closeBrace)
                                         << (STypes() << T::none << T::package << T::none << T::none << T::package)
                                         << (Starts() << 0 << 11 << 12 << 0 << 4)
                                         << (Length() << 11 << 6 << 4 << 4 << 1)
                                         << (Levels() << 0 << 1 << 1 << 1 << 1);
    QTest::newRow("newcommand command") << "\\newcommand{text}{test}"
                                        << (TTypes() << T::command << T::braces << T::def << T::braces)
                                        << (STypes() << T::none << T::def << T::none << T::definition)
                                        << (Starts() << 0 << 11 << 12 << 17 )
                                        << (Length() << 11 << 6 << 4 << 6 )
                                        << (Levels() << 0 << 1 << 1 << 1 );
    QTest::newRow("newcommand command2") << "\\newcommand{\\ext}{test}"
                                         << (TTypes() << T::command << T::braces << T::def << T::braces)
                                         << (STypes() << T::none << T::def << T::none << T::definition)
                                         << (Starts() << 0 << 11 << 12 << 17 )
                                         << (Length() << 11 << 6 << 4 << 6 )
                                         << (Levels() << 0 << 1 << 1 << 1 );
    QTest::newRow("newcommand command3") << "\\newcommand{\\paragraph}{test}"
                                         << (TTypes() << T::command << T::braces << T::def << T::braces )
                                         << (STypes() << T::none << T::def << T::none << T::definition )
                                         << (Starts() << 0 << 11 << 12 << 23 )
                                         << (Length() << 11 << 12 << 10 << 6 )
                                         << (Levels() << 0 << 1 << 1 << 1 );
    QTest::newRow("newcommand multiline definition")
        << "\\newcommand{\\paragraph}{test\nasdasd\nasd}"
        << (TTypes() << T::command << T::braces << T::def << T::openBrace << T::closeBrace )
        << (STypes() << T::none << T::def << T::none << T::definition << T::definition)
        << (Starts() << 0 << 11 << 12 << 23 <<3)
        << (Length() << 11 << 12 << 10 << 5 <<1)
        << (Levels() << 0 << 1 << 1 << 1 <<1);
    QTest::newRow("newcommand nobrace") << "\\newcommand\\foo{test}"
                                        << (TTypes() << T::command << T::command << T::braces)
                                        << (STypes() << T::none << T::def << T::definition)
                                        << (Starts() << 0 << 11 << 15)
                                        << (Length() << 11 << 4 << 6)
                                        << (Levels() << 0 << 1 << 1);
    QTest::newRow("documentclass command") << "\\documentclass{text}"
                                           << (TTypes() << T::command << T::braces << T::documentclass)
                                           << (STypes() << T::none << T::documentclass << T::none)
                                           << (Starts() << 0 << 14 << 15)
                                           << (Length() << 14 << 6 << 4)
                                           << (Levels() << 0 << 1 << 1);
    QTest::newRow("text command, embedded") << "\\textbf{te\\textit{xt} bg}"
                                            << (TTypes() << T::command << T::braces << T::word << T::command << T::braces << T::word << T::word)
                                            << (STypes() << T::none << T::text << T::text << T::text << T::text << T::text << T::text)
                                            << (Starts() << 0 << 7 << 8 << 10 << 17 << 18 << 22)
                                            << (Length() << 7 << 18 << 2 << 7 << 4 << 2 << 2)
                                            << (Levels() << 0 << 1 << 1 << 1 << 2 << 2 << 1);
    QTest::newRow("text command with comment") << "\\textbf{te % bg}"
                                            << (TTypes() << T::command << T::openBrace << T::word )
                                            << (STypes() << T::none << T::text << T::text )
                                            << (Starts() << 0 << 7 << 8 )
                                            << (Length() << 7 << 4 << 2 )
                                            << (Levels() << 0 << 1 << 1 );
    QTest::newRow("graphics command") << "\\includegraphics{file}"
                                      << (TTypes() << T::command << T::braces << T::imagefile)
                                      << (STypes() << T::none << T::imagefile << T::none)
                                      << (Starts() << 0 << 16 << 17)
                                      << (Length() << 16 << 6 << 4)
                                      << (Levels() << 0 << 1 << 1);
    QTest::newRow("graphics command with option") << "\\includegraphics[opt]{file}"
                                                  << (TTypes() << T::command << T::squareBracket << T::keyVal_key << T::braces << T::imagefile)
                                                  << (STypes() << T::none << T::keyValArg << T::none << T::imagefile << T::none)
                                                  << (Starts() << 0 << 16 << 17 << 21 << 22)
                                                  << (Length() << 16 << 5 << 3 << 6 << 4)
                                                  << (Levels() << 0 << 1 << 1 << 1 << 1);
    QTest::newRow("graphics command with keyval") << "\\includegraphics[opt,opt=text]{file}"
                                                  << (TTypes() << T::command << T::squareBracket << T::keyVal_key << T::keyVal_key << T::word << T::braces << T::imagefile)
                                                  << (STypes() << T::none << T::keyValArg << T::none << T::none << T::keyVal_val << T::imagefile << T::none)
                                                  << (Starts() << 0 << 16 << 17 << 21 << 25 << 30 << 31)
                                                  << (Length() << 16 << 14 << 3 << 3 << 4 << 6 << 4)
                                                  << (Levels() << 0 << 1 << 1 << 1 << 2 << 1 << 1);
    QTest::newRow("graphics command with keyval, multi-line") << "\\includegraphics[opt,\nopt=text]{file}"
                                                  << (TTypes() << T::command << T::openSquare << T::keyVal_key << T::keyVal_key << T::word << T::closeSquareBracket << T::braces << T::imagefile)
                                                  << (STypes() << T::none << T::keyValArg << T::none << T::none << T::keyVal_val << T::keyValArg << T::imagefile << T::none)
                                                  << (Starts() << 0 << 16 << 17 << 0 << 4 << 8 << 9 << 10)
                                                  << (Length() << 16 << 5 << 3 << 3 << 4 << 1 << 6 << 4)
                                                  << (Levels() << 0 << 1 << 1 << 1 << 2 << 1 << 1 << 1);
    QTest::newRow("graphics command with keyval in braces") << "\\includegraphics[opt={text}]{file}"
                                                  << (TTypes() << T::command << T::squareBracket << T::keyVal_key << T::braces << T::word  << T::braces << T::imagefile)
                                                  << (STypes() << T::none << T::keyValArg << T::none << T::keyVal_val << T::keyVal_val << T::imagefile << T::none)
                                                  << (Starts() << 0  << 16 << 17 << 21 << 22 << 28 << 29)
                                                  << (Length() << 16 << 12 << 3  <<  6 << 4  << 6  << 4)
                                                  << (Levels() << 0  << 1  << 1  << 2  << 2  << 1  << 1);
    QTest::newRow("graphics command with keyval in braces, multiline") << "\\includegraphics[opt={\ntext\n}]{file}"
                                                  << (TTypes() << T::command << T::openSquare << T::keyVal_key << T::openBrace << T::word  << T::closeBrace << T::closeSquareBracket << T::braces << T::imagefile)
                                                  << (STypes() << T::none << T::keyValArg << T::none << T::keyVal_val << T::keyVal_val << T::keyVal_val << T::keyValArg << T::imagefile << T::none)
                                                  << (Starts() << 0  << 16 << 17 << 21 << 0 << 0 << 1 << 2 << 3 )
                                                  << (Length() << 16 << 6  << 3  <<  1 << 4 << 1 << 1 << 6 << 4 )
                                                  << (Levels() << 0  << 1  << 1  << 2  << 3 << 3 << 2 << 1 << 1 );
    QTest::newRow("listings command with defined keyval argument") << "\\lstdefinelanguage{Excel}{morekeywords={ab$c}}"
                                                                   << (TTypes() << T::command << T::braces     << T::word       << T::braces    << T::keyVal_key << T::braces )
                                                                   << (STypes() << T::none    << T::generalArg << T::generalArg << T::keyValArg << T::none       << T::definition)
                                                                   << (Starts() << 0  << 18 << 19 << 25 << 26 << 39)
                                                                   << (Length() << 18 <<  7 <<  5 << 21 << 12 <<  6)
                                                                   << (Levels() << 0  <<  1 <<  1 <<  1 <<  1 <<  3);
    QTest::newRow("listings command with defined keyval argument2") << "\\lstdefinelanguage{Excel}{morekeywords={ab$c,sd&f}}"
                                                                   << (TTypes() << T::command << T::braces     << T::word       << T::braces    << T::keyVal_key << T::braces )
                                                                   << (STypes() << T::none    << T::generalArg << T::generalArg << T::keyValArg << T::none       << T::definition)
                                                                   << (Starts() << 0  << 18 << 19 << 25 << 26 << 39)
                                                                   << (Length() << 18 <<  7 <<  5 << 26 << 12 << 11)
                                                                   << (Levels() << 0  <<  1 <<  1 <<  1 <<  1 <<  3);
    QTest::newRow("listings command with defined keyval argument multi line")
        << "\\lstdefinelanguage{Excel}{morekeywords={ab$c,\nsd&f\n}} test"
        << (TTypes() << T::command << T::braces     << T::word       << T::openBrace << T::keyVal_key << T::openBrace << T::closeBrace << T::closeBrace << T::word)
        << (STypes() << T::none    << T::generalArg << T::generalArg << T::keyValArg << T::none       << T::definition<< T::definition << T::keyValArg  << T::none)
        << (Starts() << 0  << 18 << 19 << 25 << 26 << 39 << 0 << 1 << 3)
        << (Length() << 18 <<  7 <<  5 << 20 << 12 <<  6 << 1 << 1 << 4)
        << (Levels() << 0  <<  1 <<  1 <<  1 <<  1 <<  3 << 3 << 2 << 0);
    QTest::newRow("include command") << "\\include{text dsf}"
                                     << (TTypes() << T::command << T::braces << T::file)
                                     << (STypes() << T::none << T::file << T::none)
                                     << (Starts() << 0 << 8 << 9)
                                     << (Length() << 8 << 10 << 8)
                                     << (Levels() << 0 << 1 << 1);
    QTest::newRow("include command2") << "\\include{text/dsf}"
                                      << (TTypes() << T::command << T::braces << T::file)
                                      << (STypes() << T::none << T::file << T::none)
                                      << (Starts() << 0 << 8 << 9)
                                      << (Length() << 8 << 10 << 8)
                                      << (Levels() << 0 << 1 << 1);
    QTest::newRow("include command3") << "\\include{text\\dsf}"
                                      << (TTypes() << T::command << T::braces << T::file)
                                      << (STypes() << T::none << T::file << T::none)
                                      << (Starts() << 0 << 8 << 9)
                                      << (Length() << 8 << 10 << 8)
                                      << (Levels() << 0 << 1 << 1);
    QTest::newRow("square bracket in math") << "$\\big[ \\big]$"
                                      << (TTypes() << T::command << T::command << T::command << T::command)
                                      << (STypes() << T::none << T::none << T::none << T::none)
                                      << (Starts() << 0 << 1 << 7 << 12)
                                      << (Length() << 1 << 5 << 5 << 1)  // note: the brackets are ignored, they are not part of the command
                                      << (Levels() << 0 << 0 << 0 << 0);
    QTest::newRow("unmatched square bracket in math") << "$\\big[ \\big)$"
                                      << (TTypes() << T::command << T::command << T::command << T::command)
                                      << (STypes() << T::none << T::none << T::none << T::none)
                                      << (Starts() << 0 << 1 << 7 << 12)
                                      << (Length() << 1 << 5 << 5 << 1)  // note: the brackets are ignored, they are not part of the command
                                      << (Levels() << 0 << 0 << 0 << 0);
    QTest::newRow("unused argument in text command") << "\\textbf{\\textbf} text"
                                     << (TTypes() << T::command << T::braces << T::command << T::word)
                                     << (STypes() << T::none << T::text << T::text << T::none)
                                     << (Starts() << 0 << 7 << 8 <<17)
                                     << (Length() << 7 << 9 << 7 << 4)
                                     << (Levels() << 0 << 1 << 1 << 0);
    QTest::newRow("positional optional arguments") << "\\yagding{char}[color]"
                                                  << (TTypes() << T::command << T::braces << T::word << T::squareBracket << T::word)
                                                  << (STypes() << T::none << T::generalArg << T::generalArg << T::color << T::color)
                                                  << (Starts() << 0 << 8 << 9 << 14 << 15)
                                                  << (Length() << 8 << 6 << 4 << 7 << 5)
                                                  << (Levels() << 0 << 1 << 1 << 1 << 1);
    QTest::newRow("positional optional arguments2") << "\\yagding[test]{char}[color]"
                                                   << (TTypes() << T::command << T::squareBracket << T::keyVal_key << T::braces << T::word << T::squareBracket << T::word)
                                                    << (STypes() << T::none << T::keyValArg << T::none << T::generalArg << T::generalArg << T::color << T::color)
                                                    << (Starts() << 0 << 8 << 9 << 14 << 15 << 20 << 21)
                                                    << (Length() << 8 << 6 << 4 << 6 << 4 << 7 << 5)
                                                    << (Levels() << 0 << 1 << 1 << 1 << 1 << 1 << 1);
    QTest::newRow("positional optional arguments3") << "\\yagding[test]{char} color"
                                                    << (TTypes() << T::command << T::squareBracket << T::keyVal_key << T::braces << T::word << T::word)
                                                    << (STypes() << T::none << T::keyValArg << T::none << T::generalArg << T::generalArg << T::none)
                                                    << (Starts() << 0 << 8 << 9 << 14 << 15 <<  21)
                                                    << (Length() << 8 << 6 << 4 << 6 << 4 << 5)
                                                    << (Levels() << 0 << 1 << 1 << 1 << 1 << 0);
    QTest::newRow("default overlay") << "\\againframe[<test>]"
                                                   << (TTypes() << T::command << T::squareBracket << T::overlay)
                                                   << (STypes() << T::none << T::overlay << T::none)
                                                   << (Starts() << 0 << 11 << 12)
                                                   << (Length() << 11 << 8 << 6)
                                                   << (Levels() << 0 << 1 << 1 );
    QTest::newRow("option") << "\\againframe[test]"
                                     << (TTypes() << T::command << T::squareBracket << T::keyVal_key)
                                     << (STypes() << T::none << T::keyValArg << T::none)
                                     << (Starts() << 0 << 11 << 12)
                                     << (Length() << 11 << 6 << 4)
                                     << (Levels() << 0 << 1 << 1 );

}

void LatexParsingTest::test_latexLexing() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    LatexPackage pkg_tex = loadCwlFile("tex.cwl");
    lp->commandDefs.unite(pkg_tex.commandDescriptions);
    LatexPackage pkg_listings = loadCwlFile("txs-test.cwl");
    lp->commandDefs.unite(pkg_listings.commandDescriptions);
    LatexPackage pkg_yagu = loadCwlFile("yagusylo.cwl");
    lp->commandDefs.unite(pkg_yagu.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(TTypes, types);
    QFETCH(STypes, subtypes);
    QFETCH(Starts, starts);
    QFETCH(Length, lengths);
    QFETCH(Levels, levels);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    TokenList tl;
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        tl.append(dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList>());
    }

	for(int i=0; i<tl.length(); i++){
        Token tk = tl.at(i);
        QVERIFY2(tk.type==types.value(i), QString("incorrect type at index %1:%2").arg(i).arg(lines).toLatin1());
        QVERIFY2(tk.subtype==subtypes.value(i), QString("incorrect subtype at index %1:%2").arg(i).arg(lines).toLatin1());
        QVERIFY2(tk.start == starts.value(i), QString("incorrect start (at index %1)").arg(i).toLatin1());
        QVERIFY2(tk.length == lengths.value(i), QString("incorrect length (at index %1)").arg(i).toLatin1());
        QVERIFY2(tk.level == levels.value(i), QString("incorrect level (at index %1)").arg(i).toLatin1());
    }
    QVERIFY2(tl.length() == types.length(), "missing tokens");
    delete doc;
}

void LatexParsingTest::test_findCommandWithArgsFromTL_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<TTypes>("types");
    QTest::addColumn<STypes>("subtypes");
    QTest::addColumn<Starts>("starts");
    QTest::addColumn<Length>("lengths");
    QTest::addColumn<Levels>("levels");


    QTest::newRow("newcommand command") << "\\newcommand{text}{test}"
                                        << (TTypes() << T::braces << T::braces)
                                        << (STypes() <<  T::def <<  T::definition )
                                        << (Starts() <<  11 <<  17 )
                                        << (Length() <<  6 << 6 )
                                        << (Levels() << 1 << 1 );
    QTest::newRow("newcommand command2") << "\\newcommand{\\ext}{test}"
                                         << (TTypes() << T::braces << T::braces)
                                         << (STypes() <<  T::def <<  T::definition )
                                         << (Starts() <<  11 <<  17 )
                                         << (Length() <<  6 << 6 )
                                         << (Levels() << 1 << 1 );
    QTest::newRow("newcommand command3") << "\\newcommand{\\paragraph}{test}"
                                         << (TTypes() << T::braces << T::braces)
                                         << (STypes() <<  T::def <<  T::definition )
                                         << (Starts() <<  11 <<  23 )
                                         << (Length() <<  12 << 6 )
                                         << (Levels() << 1 << 1 );
    QTest::newRow("newcommand command, no braces") << "\\newcommand text {test}"
                                        << (TTypes() << T::word << T::braces)
                                        << (STypes() <<  T::def <<  T::definition )
                                        << (Starts() <<  12 <<  17 )
                                        << (Length() <<  4 << 6 )
                                        << (Levels() << 1 << 1 );
    QTest::newRow("newcommand command, no braces2") << "\\newcommand text test"
                                        << (TTypes() << T::word << T::word)
                                        << (STypes() <<  T::def <<  T::definition )
                                        << (Starts() <<  12 <<  17 )
                                        << (Length() <<  4 << 4 )
                                        << (Levels() << 1 << 1 );
    QTest::newRow("documentclass command") << "\\documentclass{text}"
                                           << (TTypes() << T::braces )
                                           << (STypes() <<  T::documentclass )
                                           << (Starts() <<  14 )
                                           << (Length() <<  6 )
                                           << (Levels() << 1 );
    QTest::newRow("text command, embedded") << "\\textbf{te\\textit{xt} bg}"
                                            << (TTypes() << T::braces )
                                            << (STypes() <<  T::text )
                                            << (Starts() <<  7 )
                                            << (Length() <<  18 )
                                            << (Levels() << 1 );
    QTest::newRow("text command, embedded ,open") << "\\textbf{te\\textit{xt} bg"
                                            << (TTypes() << T::openBrace )
                                            << (STypes() <<  T::text )
                                            << (Starts() <<  7 )
                                            << (Length() <<  17 )
                                            << (Levels() << 1 );

}

void LatexParsingTest::test_findCommandWithArgsFromTL() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(TTypes, types);
    QFETCH(STypes, subtypes);
    QFETCH(Starts, starts);
    QFETCH(Length, lengths);
    QFETCH(Levels, levels);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();
    // first token is command
    Token tkCmd;
    TokenList args;
    int cmdStart = Parsing::findCommandWithArgsFromTL(tl, tkCmd, args, 0, false);
    QVERIFY(cmdStart==0);
    for(int i=0; i<args.length(); i++){
        Token tk = args.at(i);
        QVERIFY2(tk.type==types.value(i), QString("incorrect type at index %1:%2").arg(i).arg(lines).toLatin1());
        QVERIFY2(tk.subtype==subtypes.value(i), QString("incorrect subtype at index %1:%2").arg(i).arg(lines).toLatin1());
        QVERIFY2(tk.start == starts.value(i), QString("incorrect start (at index %1)").arg(i).toLatin1());
        QVERIFY2(tk.length == lengths.value(i), QString("incorrect length (at index %1)").arg(i).toLatin1());
        QVERIFY2(tk.level == levels.value(i), QString("incorrect level (at index %1)").arg(i).toLatin1());
    }
    QVERIFY2(args.length() == types.length(), "missing tokens");
    delete doc;
}

void LatexParsingTest::test_getArg_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<STypes>("types");
    QTest::addColumn<QStringList>("desiredResults");


    QTest::newRow("newcommand command") << "\\newcommand{text}{test}"
                                        << (STypes() <<T::def << T::definition)
                                        << (QStringList() <<"text" <<"test");
    QTest::newRow("newcommand command2") << "\\newcommand{\\ext}{test}"
                                         <<  (STypes() <<T::def << T::definition)
                                         << (QStringList() <<"\\ext"<<"test");
    QTest::newRow("newcommand command, no braces") << "\\newcommand text {test}"
                                        <<  (STypes() <<T::def<< T::definition)
                                        << (QStringList() <<"text"<<"test");
    QTest::newRow("newcommand command, no braces2") << "\\newcommand text test"
                                        <<  (STypes() <<T::def<< T::definition) << (QStringList() <<"text"<< "test");
    QTest::newRow("documentclass command") << "\\documentclass{text}"
                                           <<  (STypes() << T::documentclass )<< (QStringList() <<"text");
    QTest::newRow("text command, embedded") << "\\textbf{te\\textit{xt} bg}"
                                            << (STypes() <<T::text) << (QStringList() <<"te\\textit{xt} bg");
    QTest::newRow("text command, embedded ,open") << "\\textbf{  te\\textit{xt} bg"
                                            <<   (STypes() <<T::text) << (QStringList() <<"  te\\textit{xt} bg");
    QTest::newRow("text command, multi-line") << "\\textbf{  te\n bg}"
                                            <<   (STypes() <<T::text) << (QStringList() <<"  te bg");
    QTest::newRow("text command, multi-line, with comment") << "\\textbf{  te%abc\n bg}"
                                            <<   (STypes() <<T::text) << (QStringList() <<"  te bg");
    QTest::newRow("text command, multi-lines,with comment") << "\\textbf{  te\n bg%abc\nas}"
                                            <<   (STypes() <<T::text) << (QStringList() <<"  te bgas");
    QTest::newRow("text command, multi-lines") << "\\textbf{  te\n bg\nas}"
                                            <<   (STypes() <<T::text) << (QStringList() <<"  te bg as");
    QTest::newRow("usepackage command, multiline") << "\\usepackage{siunitx,\ntest2}"
                                            <<   (STypes() <<T::package) << (QStringList() <<"siunitx,test2");
    QTest::newRow("usepackage command, multiline with comment") << "\\usepackage{siunitx,%abc\ntest2}"
                                            <<   (STypes() <<T::package) << (QStringList() <<"siunitx,test2");
    QTest::newRow("usepackage command, multilines") << "\\usepackage{siunitx,\ntest2,\ntest3}"
                                            <<   (STypes() <<T::package) << (QStringList() <<"siunitx,test2, test3");
    QTest::newRow("usepackage command, multilines with comment") << "\\usepackage{siunitx,\ntest2,%abc\ntest3}"
                                            <<   (STypes() <<T::package) << (QStringList() <<"siunitx,test2,test3");

}

void LatexParsingTest::test_getArg() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(STypes, types);
    QFETCH(QStringList, desiredResults);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();
    // first token is command
    Token tkCmd;
    TokenList args;
    Parsing::findCommandWithArgsFromTL(tl, tkCmd, args, 0, false);
    for(int i=0;i<types.length();i++){
        QString result = Parsing::getArg(args, types.at(i));
        QCOMPARE(result, desiredResults.at(i));
    }
    delete doc;
}

void LatexParsingTest::test_getArg2_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<ATypes>("types");
    QTest::addColumn<QList<int> >("nr");
    QTest::addColumn<QStringList>("desiredResults");


    QTest::newRow("newcommand command") << "\\newcommand{text}{test}"
                                        << (ATypes() <<ArgumentList::Mandatory << ArgumentList::Mandatory<<ArgumentList::MandatoryWithBraces<<ArgumentList::MandatoryWithBraces)
                                        << (QList<int>()<<0<<1<<0<<1)
                                        << (QStringList() <<"text" <<"test"<<"text"<<"test");
    QTest::newRow("newcommand command2") << "\\newcommand{\\ext}{test}"
                                         <<  (ATypes() <<ArgumentList::Mandatory << ArgumentList::Mandatory)
                                         << (QList<int>()<<0<<1)
                                         << (QStringList() <<"\\ext"<<"test");
    QTest::newRow("newcommand command, no braces") << "\\newcommand text {test}"
                                        <<  (ATypes() <<ArgumentList::Mandatory<< ArgumentList::Mandatory<<ArgumentList::MandatoryWithBraces<< ArgumentList::MandatoryWithBraces)
                                        << (QList<int>()<<0<<1<<0<<1)
                                        << (QStringList() <<"text"<<"test"<<""<<"");
    QTest::newRow("newcommand command, no braces2") << "\\newcommand text test"
                                        <<  (ATypes() <<ArgumentList::Mandatory<< ArgumentList::Mandatory)
                                        << (QList<int>()<<0<<1)
                                        << (QStringList() <<"text"<< "test");
    QTest::newRow("newcommand command, no braces3") << "\\newcommand{text} test"
                                        <<  (ATypes() <<ArgumentList::Mandatory<< ArgumentList::Mandatory<<ArgumentList::MandatoryWithBraces<< ArgumentList::MandatoryWithBraces)
                                        << (QList<int>()<<0<<1<<0<<1)
                                        << (QStringList() <<"text"<< "test"<<"text"<< "");
    QTest::newRow("documentclass command") << "\\documentclass{text}"
                                           <<  (ATypes() << ArgumentList::Mandatory )
                                           << (QList<int>()<<0)
                                           << (QStringList() <<"text");
    QTest::newRow("text command, embedded") << "\\textbf{te\\textit{xt} bg}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"te\\textit{xt} bg");
    QTest::newRow("text command, embedded ,open") << "\\textbf{  te\\textit{xt} bg"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te\\textit{xt} bg ");
    QTest::newRow("text command, multi-line") << "\\textbf{  te\nasdasd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd");
    QTest::newRow("text command, multi-line2") << "\\textbf{  te\nasdasd"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd ");
    QTest::newRow("text command, multi-line3") << "\\textbf{  te\nasdasd\n op\n\n}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd  op ");
    QTest::newRow("text command, multi-line4") << "\\textbf{  te\nasdasd\n op\n\ndd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd  op dd");
    QTest::newRow("text command, multi-line5") << "\\textbf{  te\nasdasd\n op\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd  op dd");
    QTest::newRow("text command, multi-line6") << "\\textbf{  te\nasdasd\n op\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd  op ");
    QTest::newRow("text command, multi-line7") << "\\textbf{  te\nasdasd\n op\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndd}" // runawaylimit=30 assumed
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd  op ");
    QTest::newRow("text command, multi-line,runaway") << "\\textbf{  te\nasdasd\n op\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ndd}" // runawaylimit=30 assumed
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd  op ");
    QTest::newRow("optional argument") << "\\section[ab]{text}"
                                            << (ATypes() <<ArgumentList::Mandatory<<ArgumentList::Optional<<ArgumentList::MandatoryWithBraces)
                                            << (QList<int>()<<0<<0<<0)
                                            << (QStringList() <<"text"<<"ab"<<"text");
    QTest::newRow("optional argument2") << "\\section[ab\ncd]{text}"
                                            << (ATypes() <<ArgumentList::Mandatory<<ArgumentList::Optional)
                                            << (QList<int>()<<0<<0)
                                            << (QStringList() <<"text"<<"ab cd");
    QTest::newRow("optional argument, with comment") << "\\section[ab% sdfg\ncd]{text}"
                                            << (ATypes() <<ArgumentList::Mandatory<<ArgumentList::Optional)
                                            << (QList<int>()<<0<<0)
                                            << (QStringList() <<"text"<<"abcd");
    QTest::newRow("optional argument3") << "\\section[]{text}"
                                            << (ATypes() <<ArgumentList::Mandatory<<ArgumentList::Optional)
                                            << (QList<int>()<<0<<0)
                                            << (QStringList() <<"text"<<"[]");
    QTest::newRow("optional argument4") << "\\section{text}"
                                            << (ATypes() <<ArgumentList::Mandatory<<ArgumentList::Optional)
                                            << (QList<int>()<<0<<0)
                                            << (QStringList() <<"text"<<"");
    QTest::newRow("multi-line") << "\\textbf\n\n{  te\nasdasd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd");
    QTest::newRow("text command, multi-line with comment") << "\\textbf{  te % Hello\nasdasd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd");
    QTest::newRow("text command, multi-line with comment2") << "\\textbf{  te % Hello\nasda% sdfsdf\nsd}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"  te asdasd");
    QTest::newRow("text command, multi-line with optional arg") << "\\usepackage[abc,\n cde]\n{hyperref}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"hyperref");
    QTest::newRow("text command, multi-line with optional arg with unknown command") << "\\usepackage[abc,\n \\cde]\n{hyperref}"
                                            << (ATypes() <<ArgumentList::Mandatory)
                                            << (QList<int>()<<0)
                                            << (QStringList() <<"hyperref");

}

void LatexParsingTest::test_getArg2() {
    int storeRUNAWAYLIMIT=ConfigManager::RUNAWAYLIMIT;
    ConfigManager::RUNAWAYLIMIT=30;
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(ATypes, types);
    QFETCH(QList<int>, nr);
    QFETCH(QStringList, desiredResults);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();
    // first token is command
    Token tkCmd;
    TokenList args;
    Parsing::findCommandWithArgsFromTL(tl, tkCmd, args, 0, false);
    for(int i=0; i < types.length(); i++){
        ArgumentList::ArgType type = ArgumentList::ArgType(types.at(i));
        QString result = Parsing::getArg(args, dlh, nr.at(i), type);
        QCOMPARE(result, desiredResults.at(i));
    }
    delete doc;
    ConfigManager::RUNAWAYLIMIT=storeRUNAWAYLIMIT;
}

void LatexParsingTest::test_getArgContent_data()
{
    QTest::addColumn<QString>("lines");
    QTest::addColumn<STypes>("types");
    QTest::addColumn<STypes>("subtypes");
    QTest::addColumn<int>("pos");
    QTest::addColumn<int>("level");

    QTest::newRow("newcommand command") << "\\newcommand{text}{test}"
                                        << (TTypes() <<T::braces << T::def << T::braces)
                                        << (STypes() <<T::def << T::none << T::definition)
                                        << 0 << 1;
    QTest::newRow("newcommand command2") << "\\newcommand{\\ext}{test}"
                                         <<  (STypes() <<T::braces << T::def << T::braces)
                                         <<  (STypes() <<T::def << T::none << T::definition)
                                         << 0 << 1;
    QTest::newRow("newcommand command, no braces") << "\\newcommand text {test}"
                                                   <<  (STypes() <<T::word << T::braces)
                                                   <<  (STypes() <<T::def << T::definition)
                                                   << 0 << 1;
    QTest::newRow("newcommand command, no braces2") << "\\newcommand text test"
                                                    <<  (TTypes() << T::word << T::word)
                                                    <<  (STypes() << T::def << T::definition)
                                                    << 0 << 1;
    QTest::newRow("newcommand command with trail") << "\\newcommand{text}{test} abc"
                                        << (TTypes() <<T::braces << T::def << T::braces)
                                        << (STypes() <<T::def << T::none << T::definition)
                                        << 0 << 1;
    QTest::newRow("newcommand command with trailing braces") << "\\newcommand{text}{test} {abc}"
                                                   << (TTypes() <<T::braces << T::def << T::braces)
                                                   << (STypes() <<T::def << T::none << T::definition)
                                                   << 0 << 1;
}

void LatexParsingTest::test_getArgContent()
{
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(TTypes, types);
    QFETCH(STypes, subtypes);
    QFETCH(int, pos);
    QFETCH(int, level);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();
    // first token is command
    Token tkCmd;
    TokenList args;
    TokenList result=Parsing::getArgContent(tl, pos,level);
    QCOMPARE(result.length(), types.length());
    for(int i=0;i<result.length();++i){
        QCOMPARE(result.at(i).type, types.at(i));
        QCOMPARE(result.at(i).subtype, subtypes.at(i));
    }
    delete doc;
}

void LatexParsingTest::test_getTokenAtCol_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<QList<int> >("nr");
    QTest::addColumn<TTypes>("desiredResults");
    QTest::addColumn<TTypes>("desiredResults2"); // with option first=true


    QTest::newRow("simple") << "bummerang  \\test"
                            << (QList<int>()<<0<<4<<9<<10<<11<<12<<16)
                            << (TTypes() << T::word << T::word << T::word << T::none << T::commandUnknown << T::commandUnknown << T::commandUnknown)
                            << (TTypes() << T::word << T::word << T::word << T::none << T::commandUnknown << T::commandUnknown << T::commandUnknown)
                            ;

    QTest::newRow("simple2") << "abc\\test"
                            << (QList<int>()<<3)
                            << (TTypes() << T::commandUnknown )
                            << (TTypes() << T::word )
                            ;
    QTest::newRow("nested1") << "\\textbf{abc}"
                            << (QList<int>()<<3<<7<<8<<11<<12)
                            << (TTypes() << T::command << T::braces  << T::word   << T::word   << T::braces )
                            << (TTypes() << T::command << T::command << T::braces << T::braces << T::braces )
                            ;
    QTest::newRow("nested2") << "\\section[abc\\textbf{abc}]{abc}"
                            << (QList<int>()<<3<<8<<14<<20<<21)
                            << (TTypes() << T::command << T::squareBracket  << T::command       << T::word          << T::word )
                            << (TTypes() << T::command << T::command        << T::squareBracket << T::squareBracket << T::squareBracket )
                            ;

}

void LatexParsingTest::test_getTokenAtCol() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(QList<int>, nr);
    QFETCH(TTypes, desiredResults);
    QFETCH(TTypes, desiredResults2); // with option first=true

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();

    for(int i=0; i < nr.length(); i++){
        int p = Parsing::getTokenAtCol(tl, nr.at(i));
        Token tk = Parsing::getTokenAtCol(dlh, nr.at(i));
        QVERIFY(tl.value(p) == tk); // cross check the two almost identical functions
        QVERIFY2(tk.type==desiredResults.at(i), QString("incorrect type at index %1:%2").arg(i).arg(lines).toLatin1());
        p = Parsing::getTokenAtCol(tl, nr.at(i), true);
        tk = Parsing::getTokenAtCol(dlh, nr.at(i), true);
        QVERIFY(tl.value(p) == tk); // cross check the two almost identical functions
        QVERIFY2(tk.type==desiredResults2.at(i), QString("incorrect type at index %1:%2").arg(i).arg(lines).toLatin1());
    }
    delete doc;
}

void LatexParsingTest::test_getCommandFromToken_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<int >("nr");
    QTest::addColumn<int >("lineNr");
    QTest::addColumn<QString>("desiredResult");


    QTest::newRow("simple") << "bummerang  \\test"
                            << 0 << 0
                            << "";

    QTest::newRow("simple2") << "bummerang  \\section{abc}"
                            << 3 << 0
                            << "\\section";

    QTest::newRow("simple3") << "bummerang  \\section{abc cde}"
                            << 4 << 0
                            << "\\section";

    QTest::newRow("simple4") << "bummerang  \\section{abc cde}"
                            << 2 << 0
                            << "\\section";
    QTest::newRow("simple5") << "bummerang  \\section abc cde"
                            << 2 << 0
                            << "\\section";
    QTest::newRow("simple6") << "bummerang  \\section abc cde"
                            << 3 << 0
                            << "";
    QTest::newRow("simple7") << "bummerang  \\section{abc {cde}}"
                            << 3 << 0
                            << "\\section";
    QTest::newRow("simple8") << "bummerang  \\section{abc {cde}}"
                            << 4 << 0
                            << "\\section";
    QTest::newRow("optonal") << "bummerang  \\section[ab ab]{abc cde}"
                            << 2 << 0
                            << "\\section";
    QTest::newRow("optonal2") << "bummerang  \\section[ab ab]{abc cde}"
                            << 3 << 0
                            << "\\section";
    QTest::newRow("optonal3") << "bummerang  \\section[ab ab]{abc cde}"
                            << 4 << 0
                            << "\\section";
    QTest::newRow("optonal4") << "bummerang  \\section[ab ab]{abc cde}"
                            << 5 << 0
                            << "\\section";
    QTest::newRow("optonal5") << "bummerang  \\section[ab ab]{abc cde}"
                            << 6 << 0
                            << "\\section";
    QTest::newRow("nested") << "bummerang  \\section{abc \\textbf{cde}}"
                            << 3 << 0
                            << "\\section";
    QTest::newRow("nested1") << "bummerang  \\section{abc \\textbf{cde}}"
                            << 4 << 0
                            << "\\section";
    QTest::newRow("nested2") << "bummerang  \\section{abc \\textbf{cde}}"
                            << 5 << 0
                            << "\\textbf";
    QTest::newRow("nested3") << "bummerang  \\section{abc \\textbf{cde}}"
                            << 6 << 0
                            << "\\textbf";
    QTest::newRow("multi arg") << "bummerang  \\newcommand{abc}{def}"
                            << 2 << 0
                            << "\\newcommand";
    QTest::newRow("multi arg2") << "bummerang  \\newcommand{abc}{def}"
                            << 3 << 0
                            << "\\newcommand";
    QTest::newRow("multi arg3") << "bummerang  \\newcommand{abc}{def}"
                            << 4 << 0
                            << "\\newcommand";
    QTest::newRow("multi arg5") << "bummerang  \\newcommand{abc}{def} werd"
                            << 5 << 0
                            << "";
    QTest::newRow("multi arg6") << "bummerang  \\newcommand{abc} def werd"
                            << 4 << 0
                            << "\\newcommand";
    QTest::newRow("keyval") << "bummerang  \\includegraphics[width=4cm]{abc} def werd"
                            << 3 << 0
                            << "\\includegraphics";
    QTest::newRow("keyval2") << "bummerang  \\includegraphics[width=4cm]{abc} def werd"
                            << 4 << 0
                            << "\\includegraphics";
    QTest::newRow("multi-line") << "bummerang  \\section{abc\n cde}"
                            << 0 << 1
                            << "\\section";


}

void LatexParsingTest::test_getCommandFromToken() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(int, nr);
    QFETCH(int, lineNr);
    QFETCH(QString, desiredResult);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(lineNr).handle();
    TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();

    QString result = Parsing::getCommandFromToken(tl.value(nr,Token()));

    QCOMPARE(result, desiredResult);

    delete doc;
}

void LatexParsingTest::test_getContext_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<int >("nr");
    QTest::addColumn<TTypes>("desiredResults");
    QTest::addColumn<STypes>("types");

    QTest::newRow("simple") << "bummerang"
                            << 2
                            << (TTypes() << T::word)
                            << (STypes() << T::none);
    QTest::newRow("command") << "\\section{abc}"
                            << 10
                            << (TTypes() << T::command <<T::braces<<T::word)
                            << (STypes() << T::none << T::title<<T::title);
    QTest::newRow("command without braces") << "\\section abc"
                            << 10
                            << (TTypes() << T::command << T::word)
                            << (STypes() << T::none << T::title);
    QTest::newRow("command with optional arg") << "\\section[fds]{abc}"
                            << 10
                            << (TTypes() << T::command << T::squareBracket<<T::word)
                            << (STypes() << T::none << T::shorttitle<<T::shorttitle);
    QTest::newRow("command with keyval") << "\\includegraphics[width=4cm]{abc}"
                            << 18
                            << (TTypes() << T::command << T::squareBracket<<T::keyVal_key)
                            << (STypes() << T::none << T::keyValArg<<T::none);
    QTest::newRow("command with keyval2") << "\\includegraphics[width=4cm]{abc}"
                            << 23
                            << (TTypes() << T::command << T::squareBracket)
                            << (STypes() << T::none << T::keyValArg);
                            //<< (TTypes() << T::command << T::squareBracket<<T::keyVal_key<<T::width) // this may be the desired outcome
                            //<< (STypes() << T::none << T::keyValArg<<T::none<<T::keyVal_val);
    QTest::newRow("command with keyval3") << "\\includegraphics[width=4cm]{abc}"
                            << 24
                            << (TTypes() << T::command << T::squareBracket<<T::keyVal_key<<T::width)
                            << (STypes() << T::none << T::keyValArg<<T::none<<T::keyVal_val);
    QTest::newRow("command with keyval4") << "\\includegraphics[width=4cm,rotate]{abc}"
                            << 28
                            << (TTypes() << T::command << T::squareBracket<<T::keyVal_key)
                            << (STypes() << T::none << T::keyValArg<<T::none);
    QTest::newRow("command with empty keyval") << "\\includegraphics[width=]{abc}"  // #4017
                            << 23
                            << (TTypes() << T::command << T::squareBracket<<T::keyVal_key<<T::keyVal_val)
                            << (STypes() << T::none << T::keyValArg<<T::none<<T::keyVal_val);
    QTest::newRow("command with keyval as defined argument") << "\\lstdefinelanguage{Excel}{morekeywords={ab$c}}"
                                          << 41
                                                             << (TTypes() << T::command << T::braces<<T::keyVal_key<<T::braces)
                                                             << (STypes() << T::none << T::keyValArg<<T::none<<T::definition);
    QTest::newRow("command with keyval as label") << "\\lstdefinelanguage{Excel}{label=test}"
                                                             << 35
                                                             << (TTypes() << T::command << T::braces<<T::keyVal_key<<T::label)
                                                             << (STypes() << T::none << T::keyValArg<<T::none<<T::keyVal_val);
    QTest::newRow("following command") << "bummerang\\text" // command after word, #3967
                            << 9
                            << (TTypes() << T::word)
                            << (STypes() << T::none);
    // stacked commands, keyval with arguments which are comma separated (#4074)
    QTest::newRow("command with keyval with command with comma separated arguments")
        << "\\mycommand{note=\\cite{abc}}"
        << 23
        << (TTypes() << T::command << T::braces<<T::keyVal_key<<T::command<<T::braces<<T::bibItem)
        << (STypes() << T::none << T::keyValArg<<T::none<<T::keyVal_val<<T::bibItem<<T::none);



}

void LatexParsingTest::test_getContext() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    LatexPackage pkg_listings = loadCwlFile("txs-test.cwl");
    lp->commandDefs.unite(pkg_listings.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(int, nr);
    QFETCH(TTypes, desiredResults);
    QFETCH(STypes, types);

    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    //TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();

    TokenStack result = Parsing::getContext(dlh,nr);

    for(int k=0;k<result.size();k++){
        if(k>=desiredResults.size()){
            continue;
        }
        QVERIFY2(result.at(k).type==desiredResults.at(k), QString("incorrect type at index %1:%2").arg(k).arg(lines).toLatin1());
        QVERIFY2(result.at(k).subtype==types.at(k), QString("incorrect subtype at index %1:%2").arg(k).arg(lines).toLatin1());
    }
    QVERIFY2(result.size()==desiredResults.size(), QString("incorrect stacksize: %1:%2").arg(result.size()).arg(desiredResults.size()).toLatin1());

    delete doc;
}

void LatexParsingTest::test_getCompleterContext_data() {
    QTest::addColumn<QString>("lines");
    QTest::addColumn<int >("nr");
    QTest::addColumn<EnumsTokenType::TokenType >("desiredResult");

    QTest::newRow("simple") << "bummerang"
                            << 2
                            <<  EnumsTokenType::word;
    QTest::newRow("command") << "\\section{abc}"
                            << 10
                            << EnumsTokenType::title;
    QTest::newRow("command without braces") << "\\section abc"
                            << 10
                            << EnumsTokenType::title;
    QTest::newRow("command with optional arg") << "\\section[fds]{abc}"
                            << 10
                            << EnumsTokenType::shorttitle;
    QTest::newRow("command with keyval") << "\\includegraphics[width=4cm]{abc}"
                            << 18
                            << EnumsTokenType::keyVal_key;
    QTest::newRow("command with keyval2") << "\\includegraphics[width=4cm]{abc}"
                            << 24
                            << EnumsTokenType::width;
    QTest::newRow("length") << "\\hspace{4cm}"
                            << 9
                            << EnumsTokenType::width;



}

void LatexParsingTest::test_getCompleterContext() {
    QSharedPointer<LatexParser> lp = QSharedPointer<LatexParser>::create();
    *lp=LatexParser::getInstance();
    LatexPackage pkg_graphics = loadCwlFile("graphicx.cwl");
    lp->commandDefs.unite(pkg_graphics.commandDescriptions);
    QFETCH(QString,lines);
    QFETCH(int, nr);
    QFETCH(EnumsTokenType::TokenType, desiredResult);


    QDocument *doc = new QDocument();
    doc->setText(lines, false);
    for(int i=0; i<doc->lines(); i++){
        QDocumentLineHandle *dlh = doc->line(i).handle();
        Parsing::simpleLexLatexLine(dlh);
    }
    TokenStack stack;
    CommandStack commandStack;
    for(int i=0; i<doc->lines(); i++){
            QDocumentLineHandle *dlh = doc->line(i).handle();
            Parsing::latexDetermineContexts2(dlh, stack, commandStack, lp);
    }
    QDocumentLineHandle *dlh = doc->line(0).handle();
    //TokenList tl= dlh->getCookieLocked(QDocumentLine::LEXER_COOKIE).value<TokenList >();

    EnumsTokenType::TokenType result = Parsing::getCompleterContext(dlh, nr);

    QCOMPARE(result, desiredResult);

    delete doc;
}

#endif


