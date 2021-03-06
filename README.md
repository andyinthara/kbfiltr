Windows keyboard filter driver


the config file is C:\Windows\kbfiltr.txt


which looks like this:

" Non-obvious key reference =================================================

" A = K_LALT
" B = K_BACKSPACE
" C = K_LCTRL
" D = K_DEL
" E = K_ESC
" F = K_CAPSLOCK
" G = K_RSHIFT
" I = K_LSHIFT
" H = K_LEFT
" J = K_DOWN
" K = K_UP
" L = K_RIGHT
" M = K_INS
" N = K_ENTER
" O = K_HOME
" P = K_END
" Q = K_COMMAND
" R = K_RSHIFT
" S = K_SPACE
" T = K_TAB
" U = K_PGUP
" V = K_PGDN
" W = K_LWIN
" X = K_NUMLOCK
" Z = K_SCROLLLOCK
" binding starting with ~ is an internal command
"
" ~ System parameters 
" ------------------------------------------------------------------------------
" logic setting values: 1 = ON, 0 = OFF
~t 1	" key hold time to register a binding
~d 11	" key press interval to register double tapping 
~r 28	" key hold time to register a key repeat 
~o 75	" key hold time to timeout key binding
~s 0	" safe mode while loading config to prevent crash
~c 0	" convert capslock to left shift , functional but obsolete as we can now use single key remaps
~l *	" starting with bindings for all layers
r0 ~r 1	" reload config
o0 ~r 1	" reload config
00 ~r 1	" reload config 

" single key remaps
" ------------------------------------------------------------------------------
F W		" capslock to Win
"[ N	" Enter 
"] B	" Backspace 



" Q for command combos
" ------------------------------------------------------------------------------
Q0 ~l 0	" switch to layer 0
Q1 ~l 1	" switch to layer 1
Q2 ~l 2	" switch to layer 2
Q3 ~l 3	" switch to layer 3

~Q a12345s6789a



" r for changing layers
" ------------------------------------------------------------------------------
qw ~l 0 " layer 0
"qk ~l 1 " layer 1
"ql ~l 2 " layer 2
"q; ~l 3 " layer 3
"[ ~l 1 " layer 1



" r for function keys 
" ------------------------------------------------------------------------------
rj !	" F1
rk @	" F2
rl #	" F3

ru $	" F4
ri %	" F5
ro ^	" F6

r7 &	" F7
r8 *	" F8
r9 (	" F9

r; )	" F10 
rh _	" F11
ry +	" F12




" D for Digits 
" ------------------------------------------------------------------------------
dj 1
dk 2
dl 3
d; 4

du 5
di 6
do 7
dp 8

dn 9
dm 0
dS 0

d' 9
d. .
d, ,

dh B	" Backspace

bj 1
bk 2
bl 3
b; 4

bu 5
bi 6
bo 7
bp 8

bn 9
bm 0



" A for windows 
" ------------------------------------------------------------------------------
aS C%	" alternative F5
an WJ	" Win + Down = Minimize 
am WK	" Win + Up = Maximize


ah A$	" alt + f4 = Close

aj W1	" Win + 1
ak W2
al W3
a; W4

ay WK	" Win + Up = Maximize
au W5
ai W6
ao W7
ap W8

a9 W9

ag Cg	" Ctrl + G entering VM
af CA	" Ctrl + ALT exiting VM

a' AT	" Alt -



" z for window movement
" ------------------------------------------------------------------------------
zj WJ	" Win + Down = Minimize 
zk WK	" Win + Up = Maximize
zh WH	" Win + Left
zl WL	" Win + Right
zu C+	" Ctrl + F12 



" q for tab control 
" ------------------------------------------------------------------------------
qj C1	" ctrl + 1
qk C2	" ctrl + 2
ql C3	" ctrl + 3
q; C4	" ctrl + 4

qu C5	" ctrl + 5
qi C6	" ctrl + 6
qo C7	" ctrl + 7
qp C8	" ctrl + 8

qn C9	" ctrl + 9

qh Cw	" ctrl + w close tab
qS Ct	" ctrl + t new tab



" c or t for input language selection
" ------------------------------------------------------------------------------
Qe IA1	" alt + shift + 1 = EN
Ql IA2	" alt + shift + 2 = LA 

~Q e0e	" change to EN layout and layer 0
cj IA1	" English key layout

~Q l2l	" change to LA layout and layer 2
ck IA2	" Lao key layout

cl IA3	" alt + shift + 3 = TH 

c; F	" Capslock



" F for brackets and punctuations 
" ------------------------------------------------------------------------------
fn -	" -
fm G-	" _

fj G9	" (
f; G0	" )

fk '	" ' 
fl G'	" " 

fu [	" [
fp ]	" ] 

fi G[	" {
fo G]	" }

fy /	" /
f[ \	" \ 

fh G,	" <
f' G.	" >

f, G,	" <
f. G.	" >

f/ G/	" ?


f7 \
f8 /
f9 .



" S for symbols
" ------------------------------------------------------------------------------
sj G1	" !
sk G2	" @
sl G3	" #
s; G4	" $


su G5	" %
si G6	" ^
so G7	" &
sp G8	" *

sh G/	" ?

sy G\	" |

s' `
sm G`	" ~

s7 \
s8 /
s9 .



" a and ; (pinky pedal) editing, control, whitespace 
" ------------------------------------------------------------------------------
;q T	" Tab |-|
;W AT	" Alt Tab 

;w G-	" _
;e -	" -
;r =	" =
;t G=	" +

;y G7	" &
;u /	" /
;i \	" \
;o G/	" ?
;p ~p	" Pause/Resume keyboard

ac Cc	" Ctrl + C
" W; G;	" : 
;s ,	" ,
;d .	" .
;f E	" Esc
;g D	" Del

;S N	" Enter

;h B	" Backspace <--
;j N	" Enter
;n G'	" "
;m G-	" _
;k .	" .
;l ,	" ,

;a Ca	" Ctrl + A 
;z Cz	" undo
;x Cx	" cut
;c Cc	" copy
;v Cv	" paste
;b Cb	" bold



" long press versions
" ------------------------------------------------------------------------------
qq Iq	" cap Q 
ww Iw
ee Ie
rr Ir
tt It
yy Iy
uu Iu
ii Ii
oo Io
pp Ip

aa Ia	" cap A
ss Is	" cap S
dd Id	" cap D 
ff If
gg Ig

zz Iz
xx Ix
cc Ic
vv Iv
bb Ib
nn In
mm Im

;; I;	" : 

" Space for shift
" ------------------------------------------------------------------------------
Sq Iq	" cap Q 
Sq Iq	" cap Q 
Sw Iw
Se Ie
Sr Ir
St It
Sy Iy
Su Iu
Si Ii
So Io
Sp Ip

Sa Ia	" cap A
Ss Is	" cap S
Sd Id	" cap D 
Sf If
Sg Ig

Sh Ih
Sj Ij
Sk Ik
Sl Il
S; I;	" : 

Sz Iz
Sx Ix
Sc Ic
Sv Iv
Sb Ib
Sn In
Sm Im




" v navigation
" ------------------------------------------------------------------------------
vh H	" Left 
vj J	" Down
vk K	" Up
vl L	" Right
v; N	" Enter

vy CO	" Ctrl+Home
vu O	" Home
vi V	" PgDn
vo U	" PgUp 
vp P	" End
v[ CP	" Ctrl+End


nh H	" Left 
nj J	" Down
nk K	" Up
nl L	" Right
n; N	" Enter



" w for AHK launchers and other app level shortcuts
" ------------------------------------------------------------------------------
wo CWo	" Everything search
wp Wm	" Windows Media player
w8 Wr	" WIN + R 
wi We	" WIN + E
wu Cu	" Yandex view source
wc Cc	" Ctrl + C


" t for layers
" ------------------------------------------------------------------------------
tj ~l 0
tk ~l 1
tl ~l 2
t; ~l 3


" Layer 1 Basic Vim navigation
" ------------------------------------------------------------------------------
~l 1	" layer 1
h H		" Left 
j J		" Down
k K		" Up
l L		" Right
; N		" Enter
x D		" Delete
e T		" Tab
0 O		" Home 
s; P	" End 

y CO	" Ctrl+Home
u O		" Home
i U		" Pgup
o V		" Pgdn 
p P		" End

f ~l 0	" Return to layer 0
p[ ~l 0	" Return to layer 0



" Layer 2 - speed Lao layer with keyhold layers
" ------------------------------------------------------------------------------
~l 2	" begin layer 2
uu n
yy b
jj a
hh q
;; '
kk t



" Layer 3  
" ------------------------------------------------------------------------------
~l 3	" begin layer 3

h 0
j 1
k 2
l 3

u 4
i 5
o 6

7 7
8 8
9 9

; .

n -
m G=	" +

