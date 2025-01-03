INCLUDE-PATH post4/assert.p4

.( Exceptions ) test_group
: tw_abort" $BEEF SWAP ABORT" Should not appear.  BEEF is stack marker." ;

\ ABORT and QUIT never return to caller, cannot be caught, see
\ https://github.com/ForthHub/discussion/discussions/116#discussioncomment-3518213
\ Though a word can throw -1, -2, or -56 directly.
: tw_catch_cold
	CATCH CASE
		 -1 OF 1001 ENDOF
		 -2 OF 1002 ENDOF
		-13 OF 1013 ENDOF
		-56 OF 1056 ENDOF
	ENDCASE
;

\ ABORT caught.
\ $DEAD ' ABORT tw_catch_cold 1001 = assert $DEAD = assert

\ No ABORT" error, pass through.
t{ $DEAD 0 ' tw_abort" tw_catch_cold -> $DEAD $BEEF }t

\ ABORT" caught, no message.
t{ $DEAD 1 ' tw_abort" tw_catch_cold -> $DEAD $BEEF 1002 }t
test_group_end

.( CATCH basic ) test_group
: tw_catch_0 ['] TRUE ;
t{ ' TRUE CATCH -> TRUE 0 }t
t{ ' tw_catch_0 CATCH -> ' TRUE 0 }t
test_group_end

.( CATCH THROW ) test_group

: tw_throw_noop $CAFE 0 THROW ;
: tw_catch_noop $FEED ['] tw_throw_noop CATCH ;
t{ tw_catch_noop -> $FEED $CAFE 0 }t

: tw_throw_depth 10 20 69 THROW ;
: tw_catch_depth 1 2 ['] tw_throw_depth CATCH ;
t{ tw_catch_depth -> 1 2 69 }t

: tw_throw_stack 2DROP 2DROP 6969 THROW ;
: tw_catch_stack 1 2 3 4 ['] tw_throw_stack CATCH DEPTH >R DROP 2DROP 2DROP R> ;
\ Test suite and/or group may have adminstration data on stack at.
t{ tw_catch_stack -> 5 }t

: tw_throw_unwind 1- DUP 0> IF RECURSE ELSE $999 THROW $222 THEN ;
: tw_catch_unwind $9876 10 ['] tw_throw_unwind CATCH $111 ;
t{ tw_catch_unwind -> $9876 0 $999 $111 }t

\ GH-8
T{ 123 :NONAME 456 -1 THROW ; CATCH -> 123 -1 }T
T{ 123 :NONAME 456 ABORT ; CATCH -> 123 -1 }T

T{ 123 :NONAME 456 -2 THROW ; CATCH -> 123 -2 }T
T{ 123 :NONAME 456 ABORT" TEST ERROR" ; CATCH -> 123 -2 }T

T{ 123 :NONAME 456 1138 THROW ; CATCH -> 123 1138 }T
T{ 123 :NONAME 456 HERE THROW ; CATCH -> 123 HERE }T

\ GH-18 stack depth at time of CATCH
\ https://github.com/SirWumpus/post4/issues/18#issuecomment-2293207015
T{ ' ABORT CATCH -> -1 }T
T{ -1 ' THROW CATCH NIP -> -1 }T
T{  1 2 :NONAME ( x x -- ) 2DROP ABORT ; CATCH NIP NIP -> -1 }T

\ GH-18 nested CATCH THROW
T{ 123 :NONAME 456 [: ABORT ;] CATCH THROW ; CATCH 789 -> 123 -1 789 }T
T{ 123 :NONAME 456 [: -1 THROW ;] CATCH THROW ; CATCH 789 -> 123 -1 789 }T
T{ 123 :NONAME 456 [: ABORT ;] ['] EXECUTE CATCH THROW ; CATCH 789 -> 123 -1 789 }T
T{ 123 :NONAME 456 [: -1 THROW ;] ['] EXECUTE CATCH THROW ; CATCH 789 -> 123 -1 789 }T

\ GH-18 S" EVALUATE CATCH
T{ 123 S" ABORT" ' EVALUATE CATCH NIP NIP -> 123 -1 }T
T{ 123 S" 0 THROW" ' EVALUATE CATCH -> 123 0 }T
T{ 123 S" -1 THROW" ' EVALUATE CATCH NIP NIP -> 123 -1 }T
T{ 123 :NONAME 456 S" -1 THROW" ['] EVALUATE CATCH NIP NIP ; EXECUTE -> 123 456 -1 }T

\ GH-21
T{ 1 0 ' / CATCH NIP NIP -> -10 ( P4_THROW_DIV_ZERO) }T
T{ S" INCLUDE /tmp/XYZZY" ' EVALUATE CATCH NIP NIP -> 2 ( ENOENT) }T
T{ S" i_am_not_number" ' EVALUATE CATCH NIP NIP -> -13 ( P4_THROW_UNDEFINED) }T
test_group_end

.( -56 THROW ) test_group
\ https://github.com/ForthHub/discussion/discussions/116#discussioncomment-3541822
t{ :NONAME 123 [: 456 -56 THROW ;] CATCH ; EXECUTE -> 123 -56 }t
test_group_end

.( GH-79 ) test_group
t{ ' DROP CATCH   ->   -4 }t
\ t{ 0 ' @ CATCH .s -> 0 -9 }t
\ t{ 1 ' @ CATCH .s -> 1 -9 }t
test_group_end
