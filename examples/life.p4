\ Conway's Game Of Life
\
\ http://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
\
\ Rules
\
\ The universe of the Game of Life is an infinite two-dimensional orthogonal
\ grid of square cells, each of which is in one of two possible states, live
\ or dead. Every cell interacts with its eight neighbors, which are the cells
\ that are directly horizontally, vertically, or diagonally adjacent. At each
\ step in time, the following transitions occur:
\
\    1. Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
\    2. Any live cell with more than three live neighbours dies, as if by overcrowding.
\    3. Any live cell with two or three live neighbours lives on to the next generation.
\    4. Any dead cell with exactly three live neighbours becomes a live cell.
\
\ The initial pattern constitutes the seed of the system. The first generation
\ is created by applying the above rules simultaneously to every cell in the
\ seed - births and deaths happen simultaneously, and the discrete moment at
\ which this happens is sometimes called a tick (in other words, each generation
\ is a pure function of the one before). The rules continue to be applied
\ repeatedly to create further generations.

MARKER rm_life

#24 VALUE rows
#80 VALUE columns

CHAR # VALUE on
CHAR . VALUE off

( u -- a-addr )
: alloc ALLOCATE DROP ;

( -- u )
: /screen rows columns * CHARS ;

( -- c-addr )
: screen_alloc /screen alloc ;

screen_alloc VALUE screen0
screen_alloc VALUE screen1
screen0 VALUE screen_curr
screen1 VALUE screen_next

( c-addr char -- )
: screen_set /screen SWAP FILL ;

( caddr -- )
: screen_reset off screen_set ;

( -- )
: screen_swap
	screen_curr
	screen_next TO screen_curr
	TO screen_next
;

( screen row -- )
: .row
	columns * CHARS			\ screen row_off
	+				\ screen'
	columns CHARS TYPE		\ --
	CR
;

( screen -- )
: .screen
	rows 0 ?DO
		DUP I 			\ screen screen row
		.row			\ screen
	LOOP DROP CR			\ --
;

( row -- flag )
: ?row 0 rows WITHIN ;

( column -- flag )
: ?column 0 columns WITHIN ;

( column row -- flag )
: ?screen ?row SWAP ?column AND ;

( column row -- offset )
: row_col+ columns * + CHARS ;

( column row -- char )
: screen@ row_col+ screen_curr + C@ ;

( char column row -- )
: screen! row_col+ screen_next + C! ;

( column row -- count )
: #neighbours
	2>R 0 2R>				\ count col row
	DUP #2 +				\ count col row row+2
	SWAP 1-					\ count col row+2 row-1
	?DO
		DUP #2 +			\ count col col+2
		OVER 1-				\ count col col+2 col-1
		?DO				\ count col
			I J ?screen		\ count col flag
			IF
				I J screen@ on = 	\ count col flag
				IF SWAP 1+ SWAP THEN	\ count' col
			THEN
		LOOP
	LOOP
	DROP
;

( count -- char )
: birth
	#3 =						\ flag
	IF on ELSE off THEN				\ char
;

( count -- char )
: death
	#3 #5 WITHIN					\ flag
	IF on ELSE off THEN 				\ char
;

( -- )
: .neighbours
	rows 0 ?DO
		columns 0 ?DO
			I J #neighbours			\ count
			.
		LOOP
		CR
	LOOP
;

( -- )
: generation
	rows 0 ?DO
		columns 0 ?DO
			I J #neighbours			\ count
			I J screen@			\ count char
			on = 				\ count flag
			IF death ELSE birth THEN	\ char'
			I J screen!			\ --
		LOOP
	LOOP
;

( n -- )
: .generations
	CR ." generation #0" CR
	screen_curr .screen
	1+ 1 DO
		CR ." generation #" I . CR
		generation
		screen_swap
		screen_curr .screen
	LOOP
;

( n -- )
: generations
	PAGE
	1+ 1 DO
		0 0 AT-XY
		." generation #" I . CR
		generation
		screen_swap
		screen_curr .screen
		KEY? IF KEY DROP '\a' EMIT LEAVE THEN
		#250 MS
	LOOP
;

( -- )
: >screen
	\ First line of input defines screen width.
	CR ." Define initial life grid; dot (.) for empty cell, hash (#) live cell."
	CR ." All lines must be the same length; a short or blank line ends input."
	CR

	REFILL 0= IF EXIT THEN
	BL PARSE				\ input u
	DUP TO columns				\ input u

	0 TO rows				\ input u
	BEGIN
		rows 1+ TO rows
		/screen				\ input u size
		DUP screen_curr SWAP		\ input u size screen size
		RESIZE DROP TO screen_curr	\ input u size
		OVER -				\ input	u size'
		screen_curr + SWAP		\ input screen' u
		MOVE				\ --

		REFILL DROP
		BL PARSE			\ input u
		DUP columns <>			\ input u
	UNTIL
	2DROP					\ --

	screen_next /screen ALLOT TO screen_next
;

.( Still life and oscilators)
>screen
.................................................
.................................................
..##......###...................###...###........
..##.............................................
..............................#....#.#....#......
..............................#....#.#....#......
...##.............##..........#....#.#....#......
..#..#.....###....##............###...###........
...##.....###.......##...........................
....................##..........###...###........
..............................#....#.#....#......
...##.........................#....#.#....#......
..#..#....##..................#....#.#....#......
...#.#....#.#....................................
....#......#....................###...###........
.................................................
.................................................

10 generations

.( Gosper Glider Gun)
>screen
.................................................
.........................#.......................
.......................#.#.......................
.............##......##............##............
............#...#....##............##............
.##........#.....#...##..........................
.##........#...#.##....#.#.......................
...........#.....#.......#.......................
............#...#................................
.............##..................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................

100 generations

.( Die Hard)
>screen
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
..........................#......................
....................##...........................
.....................#...###.....................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................

140 generations

.( Acorn, takes 5206 generations to generate 633 cells including 13 escaped gliders.)
>screen
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
......................#..........................
........................#........................
.....................##..###.....................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................

300 generations

>screen
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
...................###.#.........................
...................#.............................
......................##.........................
....................##.#.........................
...................#.#.#.........................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................

300 generations

>screen
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.........................#.......................
.......................#.##......................
.......................#.#.......................
.......................#.........................
.....................#...........................
...................#.#...........................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................
.................................................

300 generations
