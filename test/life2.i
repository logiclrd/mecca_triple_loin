	PLEASE NOTE BEGINNING OF BEGINNING
        DO WRITE IN .11
        DO WRITE IN .12
        DO ,1 <- .11 BY .12
        DO ,2 <- .11 BY .12
	DO WRITE IN .13
	DO .14 <- #0
	DO .1 <- .11
	DO (1020) NEXT
	DO .4 <- .1
	DO ,3 <- .4
	DO .2 <- #0
	PLEASE STASH .2

	PLEASE NOTE INITIALIZING ,1 TO ZEROES
	DO .1 <- .11
	PLEASE COME FROM (116)
	DO .2 <- .1
	DO .1 <- .12
	PLEASE COME FROM (113)
	DO ,1 SUB .2.1 <- #0
	DO (111) NEXT
(112)	DO (2010) NEXT
(113)	PLEASE FORGET #2
(111)   DO (112) NEXT
	DO (114) NEXT
(115)	DO .1 <- .2
	DO (2010) NEXT
(116)	PLEASE FORGET #3
(114)   DO (115) NEXT
	PLEASE FORGET #2

	PLEASE NOTE INITIAL POSITION INPUT
	PLEASE COME FROM (123)
	DO WRITE IN .1
	DO (121) NEXT
	DO WRITE IN .2
(123)	DO ,1 SUB .1.2 <- #1
(122)	DO RESUME "?!1~.1'$#1"~#3
(121)   DO (122) NEXT
	PLEASE FORGET #1

	PLEASE NOTE END OF BEGINNING, BEGINNING OF MIDDLE
	DO COME FROM (298)

	PLEASE NOTE PRINTING CURRENT POSITION
	DO (300) NEXT

	PLEASE NOTE PUTTING NEXT POSITION IN ,2
	DO .1 <- .11
	DO .9 <- .1
	DO (2000) NEXT
	DO .7 <- .1
	DO (2000) NEXT
	PLEASE COME FROM (216)
	DO .5 <- .1
	DO .1 <- .12
	DO .10 <- .1
	DO (2000) NEXT
	DO .8 <- .1
	DO (2000) NEXT
	PLEASE COME FROM (213)
	DO .6 <- .1

	DO .1 <- #0
	DO .2 <- ,1 SUB .5.6
	DO (2020) NEXT
	DO .2 <- ,1 SUB .5.8
	DO (2020) NEXT
	DO .2 <- ,1 SUB .5.10
	DO (2020) NEXT
	DO .2 <- ,1 SUB .7.6
	DO (2020) NEXT
	DO .2 <- ,1 SUB .7.10
	DO (2020) NEXT
	DO .2 <- ,1 SUB .9.6
	DO (2020) NEXT
	DO .2 <- ,1 SUB .9.8
	DO (2020) NEXT
	DO .2 <- ,1 SUB .9.10
	DO (2020) NEXT
	DO :2 <- #0$#65535
	DO .1 <- "?'"V.1$,1SUB.7.8"~:2'$#3"~:2
	DO ,2 SUB .7.8 <- "?!1~.1'$#1"~#1

	DO (211) NEXT
(212)   DO .10 <- .8
	DO .8 <- .6
	DO .1 <- .6
	DO (2010) NEXT
(213)	PLEASE FORGET #2
(211)   DO (212) NEXT
	DO (214) NEXT
(215)	DO .9 <- .7
	DO .7 <- .5
	DO .1 <- .5
	DO (2010) NEXT
(216)	PLEASE FORGET #3
(214)   DO (215) NEXT
	PLEASE FORGET #2

	PLEASE NOTE COPYING ,2 BACK INTO ,1
	DO .1 <- .11
	DO (2000) NEXT
	PLEASE COME FROM (226)
	DO .2 <- .1
	DO .1 <- .12
	DO (2000) NEXT
	PLEASE COME FROM (223)
	DO ,1 SUB .2.1 <- ,2 SUB .2.1
(223)	DO (221) NEXT
(222)	DO (2000) NEXT
	DO .3 <- "?.1$#1"~"#0$#65535"
	PLEASE RESUME '?"!3~.3'~#1"$#1'~#3
(221)   DO (222) NEXT
	PLEASE FORGET #1
(226)	DO (224) NEXT
(225)	DO .1 <- .2
	DO (2000) NEXT
	DO .3 <- "?.1$#1"~"#0$#65535"
	PLEASE RESUME '?"!3~.3'~#1"$#1'~#3
(224)   DO (225) NEXT
	PLEASE FORGET #1

	PLEASE NOTE INCREMENT TIMESTEP AND TEST
	DO .1 <- .14
	DO (1020) NEXT
	DO .14 <- .1
	DO (231) NEXT
	DO (299) NEXT
(232)	DO .3 <- "?.14$.13"~"#0$#65535"
	PLEASE RESUME '?"!3~.3'~#1"$#2'~#3
(231)	DO (232) NEXT
	DO FORGET #1

	PLEASE NOTE OVERFLOW TESTING
	DO .7 <- #0
	DO .8 <- #0
	DO .1 <- .11
	DO (2000) NEXT
	DO .5 <- .1
	DO .1 <- .12
	DO (2000) NEXT
	DO COME FROM (243)
	DO .6 <- .1
	DO .2 <- ,1 SUB #2.6
	DO .1 <- .7
	DO (500) NEXT
	DO .7 <- .1
	DO .2 <- ,1 SUB .5.6
	DO .1 <- .8
	DO (500) NEXT
	DO .8 <- .1
(243)	DO (241) NEXT
(242)	DO .1 <- .6
	DO (2000) NEXT
	DO .3 <- "?.1$#1"~"#0$#65535"
	PLEASE RESUME '?"!3~.3'~#1"$#1'~#3
(241)   DO (242) NEXT
	PLEASE FORGET #1

	DO .7 <- #0
	DO .8 <- #0
	DO .1 <- .12
	DO (2000) NEXT
	DO .6 <- .1
	DO .1 <- .11
	DO (2000) NEXT
	DO COME FROM (253)
	DO .5 <- .1
	DO .2 <- ,1 SUB .5#2
	DO .1 <- .7
	DO (500) NEXT
	DO .7 <- .1
	DO .2 <- ,1 SUB .5.6
	DO .1 <- .8
	DO (500) NEXT
	DO .8 <- .1
(253)	DO (251) NEXT
(252)	DO .1 <- .5
	DO (2000) NEXT
	DO .3 <- "?.1$#1"~"#0$#65535"
	PLEASE RESUME '?"!3~.3'~#1"$#1'~#3
(251)   DO (252) NEXT
(298)	PLEASE FORGET #1

(299)	DO FORGET #1
	PLEASE NOTE END OF MIDDLE, BEGINNING OF END

	PLEASE NOTE PRINTING FINAL POSITION
	DO (300) NEXT

	PLEASE NOTE END OF END
	PLEASE GIVE UP


(300)	PLEASE NOTE POSITION OUTPUT ROUTINE
	DO READ OUT .14
	PLEASE RETRIEVE .2
	DO .6 <- .12
	PLEASE COME FROM (316)
	DO .5 <- #1
	PLEASE COME FROM (313)
	DO .1 <- .2
	DO (301) NEXT
	DO .2 <- #242
	DO (303) NEXT
(302)	PLEASE RESUME "?',1 SUB .5.6'$#1"~#3
(301)   DO (302) NEXT
	DO .2 <- #116
(303)	PLEASE FORGET #1
	DO (1010) NEXT
	DO ,3 SUB .5 <- .3
(313)	DO (311) NEXT
(312)	DO .1 <- .5
	DO (1020) NEXT
	DO .5 <- .1
	DO .3 <- "?.5$.4"~"#0$#65535"
	PLEASE RESUME '?"!3~.3'~#1"$#1'~#3
(311)   DO (312) NEXT
	DO .1 <- .2
	DO .2 <- #80
	DO (1010) NEXT
	DO ,3 SUB .4 <- .3
	PLEASE READ OUT ,3
	DO (314) NEXT
(315)	DO .1 <- .6
	DO (2010) NEXT
	PLEASE FORGET #3
(316)	DO .6 <- .1
(314)   DO (315) NEXT
	PLEASE DO STASH .2
	PLEASE RESUME #3

(500)	DO (501) NEXT
	DO .1 <- #0
	PLEASE RESUME #1
(502)   PLEASE RESUME '?.2$#2'~#3
(501)	DO (502) NEXT
	DO (2020) NEXT
	DO (503) NEXT
	PLEASE RESUME #2
(504)	DO .2 <- "?.1$#3"~"#0$#65535"
	PLEASE RESUME '?"!2~.2'~#1"$#1'~#3
(503)	DO (504) NEXT
	PLEASE FORGET #3
	DO (299) NEXT

(2010)  PLEASE ABSTAIN FROM (2004)
(2000)  PLEASE STASH .2
        DO .2 <- #1
        DO (2001) NEXT
(2001)  PLEASE FORGET #1
        DO .1 <- '?.1$.2'~'#0$#65535'
        DO (2002) NEXT
        DO .2 <- !2$#0'~'#32767$#1'
        DO (2001) NEXT
(2003)  PLEASE RESUME "?!1~.2'$#1"~#3
(2002)  DO (2003) NEXT
        PLEASE RETRIEVE .2
(2004)	PLEASE RESUME #2
	PLEASE DO REINSTATE (2004)
	PLEASE RESUME '?"!1~.1'~#1"$#2'~#6

(2020)  PLEASE STASH .2 + .3
	DO (1021) NEXT


