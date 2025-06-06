# diagram package
# Matthew Bertucci 2025/03/15 for v1.23

#include:ifthen
#include:calc
#include:tikz
#include:xstring

#keyvals:\usepackage/diagram#c
10pt
11pt
12pt
#endkeyvals

\begin{diagram}
\begin{diagram}[%<<cols>%>x%<<rows>%>]
\end{diagram}
\putsol
\normalnames
\reversednames
\Dr
\Prof
\ProfDr
\pieces{pieces}
\pieces[white+black]{pieces}
\fen{pieces}
\fen[white+black]{pieces}
\stipulation{stipulation}
\stip{stipulation}#*
\city{city}
\specialdiagnum{number}
\sourcenr{number}
\source{source}
\issue{number}
\pages{pages}
\day{day}
\month{month}
\months{months}
\year{year}
\tournament{name}
\tourn{name}#*
\award{name}
\dedication{text}
\dedic{text}#*
\condition{text}
\cond{text}#*
\twins{text}
\remark{text}
\rem{text}#*
\piecedefs{{%<piece1%>}{%<piece2%>}{%<text%>}%<;...%>}
\solution{text}
\sol{text}#*
\judgement{text}
\comment{text}
\themes{text}
\verticalcylinder
\horizontalcylinder
\noframe
\noinnerframe
\gridchess
\stdgrid#*
\diagleft
\diagcenter
\diagright
\widedias
\dianamestyle{option%keyvals}
\solnamestyle{option%keyvals}
#keyvals:\dianamestyle#c,\solnamestyle#c
fullname
sirname
short
noname
#endkeyvals
\diagnumbering{numstyle%keyvals}
\setmonthstyle{numstyle%keyvals}
#keyvals:\diagnumbering#c,\setmonthstyle#c
arabic
Roman
roman
Alph
alph
#endkeyvals
\nocomputer
\showcomputer
\notcomputerproofedsymbol#*
\computerproofedsymbol#*
\selectelchfont{type%keyvals}
#keyvals:\selectelchfont
pk
fs
#endkeyvals
\diagramx#*
\diagramxi#*
\diagramxii#*
\diagnum{number}
\diagnum[prefix]{number}
\begin{stereodiagram}
\end{stereodiagram}
\begin{spacediagram}
\end{spacediagram}
\spacelayout{type%keyvals}
#keyvals:\spacelayout
vertical
horizontal
#endkeyvals
\allwhite
\switchcolors
\begin{figurine}
\end{figurine}
\nofields{list of squares}
\nosquares{list of squares}
\fieldframe{list of fields}
\magic{list of fields}#*
\gridlines{list of lines}
\fieldtext{{%<text1%>}%<piece1,...%>}
\swL
\ssL
\wNr
\nNr
\sNr
\wGh
\nGh
\sGh
\Imi
\wC
\nC
\sC
\wE
\sE
\nE
\wX
\sX
\nX
\set
\ra
\lra
\OO
\OOO
\x
\any
\begin{arrowskip}{prefix}{suffix}
\end{arrowskip}
\DefinePieces{color letters}{piece letters}{rotation letters}
\develop
\makeaindex
\authorindex
\makesindex
\sourceindex
\maketindex
\themeindex
\solpar
\insidediagram{code}

# not in main documentation
\after{arg}#*
\authorfont#*
\awardfont#*
\cityfont#*
\Co{arg}#*
\correction{arg}#*
\dedicfont#*
\DefaultDiagramSize#*
\defaultelchfont#*
\elchfont#*
\fidealbum{arg}#*
\further#*
\ifimitator#*
\imitatorfalse#*
\imitatortrue#*
\labelfont#*
\legendfont#*
\nodiagnumbering#*
\normalboardwidth#*
\nowidedias#*
\remfont#*
\rla#*
\setboardwidth#*
\showlabel{arg}#*
\showtypis{arg}#*
\solafterdiagram#*
\solhead{arg}#*
\sourcefont#*
\spacehorizontal#*
\stipfont#*
\textproblem#*
\thediag#*
\topdist#*
\version{arg}#*
\wF#S
