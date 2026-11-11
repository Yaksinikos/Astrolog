@AD800  ; Astrolog 8.00 default settings file astrolog.as

; The contents of this file can be automatically generated with the
; "File / Save Program Settings" menu command, or with the -od command switch.

-z 1:00E                ; Default time zone     [hours W or E of UTC   ]
-z0 1          ; Default Daylight time [0 standard, 1 daylight]
-zl  11E34'30 48N08'15  ; Default location      [longitude and latitude]
-zv 505m               ; Default elevation     [in feet or meters     ]
-zf 20C                 ; Default temperature   [in Fahren. or Celsius ]
-zj "Current moment now" "Munich, Germany" ; Default name and location

-Yz 0   ; Time minute addition to be used if "now" charts are offset.
-n      ; Comment out this line to not start with chart for "now".

=s      ; Which zodiac to use       ["_s" is tropical, "=s" is sidereal]
:s Faga ; Sidereal zodiac offset    [Change "Faga" to desired ayanamsa ]
:sz     ; Zodiac display format     ["z" is sign, "d" is 0-360 deg, etc]
_sr0    ; Latitudes or declinations ["_sr0" shows lat., "=sr0" declin. ]
-A 5    ; Number of aspects         [Change "5" to desired number      ]
-c Pull ; House system              [Change "Plac" to desired system   ]
_c3     ; 3D house boundaries       ["=c3" is 3D houses, "_c3" is 2D   ]
=k      ; Ansi color text           ["=k" is color, "_k" is monochrome ]
=b0     ; Print zodiac seconds      ["_b0" to minute, "=b0" to second  ]
=b1     ; Print zodiac milliseconds ["_b1" to second, "=b1" to millisec]
_b2     ; Don't display :00 seconds ["_b2" shows anyway, "=b2" skips   ]
=b      ; Use ephemeris files       ["=b" uses them, "_b" doesn't      ]
=0b     ; Disable old calculations  ["=0b" disables them, "_0b" allows ]
_v0     ; Show average velocities   ["=v0" average, "_v0" does absolute]
=v3 9   ; Wheel subdivision type    [Change "0" to desired subdivision ]
:w 0    ; Wheel chart text rows     [Change "0" to desired wheel rows  ]
:gs 0   ; Aspect orb type           ["0" +/-, "1" app/sep, "2" wax/wan ]
:d 48   ; Searching divisions       [Change "48" to desired divisions  ]
_5      ; Transits go to chart list ["=5" sets list, "_5" does nothing ]
:I 80   ; Text screen columns       [Change "80" to desired columns    ]
_I0     ; Interpret Sabian symbols  ["=I0" for Sabian, "_I0" for normal]
-YQ 0   ; Text screen scroll limit  [Change "24" or set to "0" for none]
=Ys     ; Use plane of solar system ["_Ys" is ecliptic, "=Ys" is solar ]
=Yn     ; Which Nodes and Lilith    ["_Yn" shows mean, "=Yn" shows true]
=Yu0    ; Show eclipse information  ["=Yu0" shows, "_Yu0" doesn't show ]
=Yd     ; European date format      ["_Yd" is M/D/Y, "=Yd" is D-M-Y    ]
=Yt     ; European time format      ["_Yt" is AM/PM, "=Yt" is 24 hour  ]
=Yv     ; European length units     ["_Yv" is imperial, "=Yv" is metric]
_Yr     ; Show rounded positions    ["=Yr" rounds, "_Yr" doesn't       ]
=YC     ; Smart cusp displays       ["=YC" is smart, "_YC" is normal   ]
=YO     ; Smart copy and printing   ["=YO" does it smart, "_YO" doesn't]
=Y8     ; Clip text to end of line  ["=Y8" clips, "_Y8" doesn't clip   ]
-Ya0    ; Input character encoding  [0-3 is Default, IBM, Latin-1, UTF8]
_Yz1    ; Combine DST and time zone ["=Yz1" combines, "_Yz1" doesn't   ]
-YP 0   ; Arabic part formula       ["1" is fixed, "0" checks if night ]
=0n     ; Internet Web queries      ["=0n" disables them, "_0n" allows ]

-Yw 0.0       ; Stationary movement threshold  [0.0 is never "S"]
:pd 365.24219 ; Progression degrees per day    [365 is secondary]
:pC 1.0       ; Progressed cusp movement ratio [1.0 is quotidian]
:pO Sun       ; Solar arc based on this planet [-1 is fixed rate]
_pc           ; Solar arc recalc based on MC   [=pc recalculates]


; FILE PATHS (-Yi1 through -Yi9):
; For example, point -Yi1 to ephemeris dir, -Yi2 to font dir, etc.

-Yi1 "ephem"
-Yi2 "font"
-Yi3 "source"


; DEFAULT RESTRICTIONS:
;  0-10: Ear Sun Moo Mer Ven Mar Jup Sat Ura Nep Plu
; 11-21: Chi Cer Pal Jun Ves Nor Sou Lil For Ver EP
; 22-33: Asc 2nd 3rd Nad 5th 6th Des 8th 9th MC 11th 12th
; 34-42: Vul Cup Had Zeu Kro Apo Adm Vulk Pos
; 43-51: Hyg Pho Eri Hau Mak Gon Qua Sed Orc
; 52-78: Planetary moons
; 79-83: Planetary centers of body
; 84-133: Fixed stars

-YR 0 10     0 0 0 0 0 0 0 0 0 0 0    ; Planets
-YR 11 21    0 0 0 0 0 0 0 0 0 0 0    ; Minor planets
-YR 22 33    0 0 0 0 0 0 0 0 0 0 0 0  ; House cusps
-YR 34 42    0 0 0 0 0 0 0 0 0        ; Uranians
-YR 43 51    0 0 0 0 0 0 0 0 0        ; Dwarfs
-YR 52 78    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; Moons
-YR 79 83    1 1 1 1 1                ; Centers of body
-YR 84 108   1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; Fixed stars
-YR 109 133  1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; Fixed stars

; DEFAULT TRANSIT RESTRICTIONS:

-YRT 0 10    0 0 0 0 0 0 0 0 0 0 0    ; Planets
-YRT 11 21   0 0 0 0 0 0 0 0 0 0 0    ; Minor planets
-YRT 22 33   0 0 0 0 0 0 0 0 0 0 0 0  ; House cusps
-YRT 34 42   0 0 0 0 0 0 0 0 0        ; Uranians
-YRT 43 51   0 0 0 0 0 0 0 0 0        ; Dwarfs
-YR 52 78    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; Moons
-YR 79 83    1 1 1 1 1                ; Centers of Body
-YR 84 108   1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; Fixed stars
-YR 109 133  1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; Fixed stars

-YR0 0 0  ; Restrict sign changes, direction changes
-YR1 0 0  ; Restrict latitude direction changes, distance direction changes
-YR2 0 0  ; Restrict latitude zero node crossings, distance equivalence

-YR7 0 0 0 0 0  ; Restrict rulerships: std, esoteric, hierarch, exalt, Ray
-YRZ 0 0 0 0    ; Restrict angle events: rising, zenith, setting, nadir
-YRp 0 0        ; Restrict prime vertical: vertex, antivertex


; DEFAULT ASPECT ORBS:
;  1- 5: Con Opp Squ Tri Sex
;  6-11: Inc SSx SSq Ses Qui BQn
; 12-18: SQn Sep Nov BNv BSp TSp QNv
; 19-24: TDc Un1 Un2 Un3 Un4 Un5

-YAo 1 5     4 4 4 4 3      ; Major aspects
-YAo 6 11    3 3 3 3 2 2    ; Minor aspects
-YAo 12 18   1 1 1 1 1 1 1  ; Obscure aspects
-YAo 19 24   0.5 0.5 0.5 0.5 0.5 0.5  ; Very obscure aspects

; DEFAULT MAX PLANET ASPECT ORBS:

-YAm 0 10    360 360 360 360 360 360 360 360 360 360 360      ; Planets
-YAm 11 21   360 360 360 360 360   2   2   2 360 360   2      ; Minor planets
-YAm 22 33   360 360 360 360 360 360 360 360 360 360 360 360  ; Cusp objects
-YAm 34 42   360 360 360 360 360 360 360 360 360              ; Uranians
-YAm 43 51   360 360 360 360 360 360 360 360 360              ; Dwarfs
-YAm 84 84     2                                              ; Fixed stars

; DEFAULT PLANET ASPECT ORB ADDITIONS:

-YAd 0 10    1 1 1 0 0 0 0 0 0 0 0    ; Planets
-YAd 11 21   0 0 0 0 0 0 0 0 0 0 0    ; Minor planets
-YAd 22 33   0 0 0 0 0 0 0 0 0 0 0 0  ; Cusp objects
-YAd 34 42   0 0 0 0 0 0 0 0 0        ; Uranians
-YAd 43 51   0 0 0 0 0 0 0 0 0        ; Dwarfs
-YAd 84 84   0                        ; Fixed stars


; DEFAULT INFLUENCES:

-Yj 0 10    30 30 25 10 10 10 10 10  8  8  8     ; Planets
-Yj 11 21    6  5  5  5  5  5  5  4  4  4  4     ; Minor planets
-Yj 22 33   20 10 10 10 10 10 10 10 10 15 10 10  ; Cusp objects
-Yj 34 42    4  3  3  3  3  3  3  3  3           ; Uranians
-Yj 43 51    3  3  3  3  3  3  3  3  3           ; Dwarfs
-Yj 84 84    2                                   ; Fixed stars

-YjC 1 12   20 0 0 10 0 0 5 0 0 15 0 0  ; Houses

-YjA 1 5    1.0 0.8 0.8 0.6 0.6          ; Major aspects
-YjA 6 11   0.4 0.4 0.2 0.2 0.2 0.2      ; Minor aspects
-YjA 12 18  0.1 0.1 0.1 0.1 0.1 0.1 0.1  ; Obscure aspects

; DEFAULT TRANSIT INFLUENCES:

-YjT 0 10   10 10  4  8  9 20 30 35 40 45 50  ; Planets
-YjT 11 21  30 15 15 15 15 30 30  1  1  1  1  ; Minor planets
-YjT 34 42   2 50 50 50 50 50 50 50 50        ; Uranians
-YjT 43 51  15 30 50 50 50 50 50 50 50        ; Dwarfs
-YjT 84 84  60                                ; Fixed stars

-Yj0 20 10 15 5  ; In ruling sign, exalted sign, ruling house, exalted house
-Yj7 10 10 10 5 5 5  ; In Esoteric, Hierarchical, Ray ruling (signs, houses)


; DEFAULT RULERSHIPS & EXALTATIONS:

-YJ Sun Leo 0
-YJ Moo Can 0
-YJ Mer Gem Vir
-YJ Ven Lib Tau
-YJ Mar Ari Sco
-YJ Jup Sag Pis
-YJ Sat Cap Aqu
-YJ Ura Aqu 0
-YJ Nep Pis 0
-YJ Plu Sco 0

-YJ0 Sun Ari
-YJ0 Moo Tau
-YJ0 Mer Vir
-YJ0 Ven Pis
-YJ0 Mar Cap
-YJ0 Jup Can
-YJ0 Sat Lib
-YJ0 Ura Sco
-YJ0 Nep Can
-YJ0 Plu Aqu


; DEFAULT RAYS:

-Y7C 1 12   17 4 2 37 15 26 3 4 456 137 5 26  ; Signs
-Y7O 0 10   3 2 4 4 5 6 2 3 7 6 1             ; Planets
-Y7O 34 42  1 0 0 0 0 0 0 0 0                 ; Uranians
-Y7O 43 51  0 0 3 2 1 4 7 6 5                 ; Dwarfs


; DEFAULT COLORS:
; Black, White, Gray, LtGray, Red, Maize, Yellow, Green, Cyan, Blue, Purple,
; Magenta, Maroon, DkGreen, DkCyan, DkBlue; Element, Ray, Star, Planet;
; DkGray, Orange, Pink, Brown, Indigo, Forest, Amber, Rose, Sky, Violet

-YkO 0 10   Yel Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele      ; Planet colors
-YkO 11 21  Mag Mag Mag Mag Mag DkC DkC DkC DkC DkC DkC      ; Minor colors
-YkO 22 33  Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele  ; Cusp colors
-YkO 34 42  Pur Pur Pur Pur Pur Pur Pur Pur Pur              ; Uranian colors
-YkO 43 51  Mag Mag Pur Pur Pur Pur Pur Pur Pur              ; Dwarf colors
-YkO 52 63  Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla  ; Moons
-YkO 64 75  Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla  ; Moons
-YkO 76 83  Pla Pla Pla Pla Pla Pla Pla Pla                  ; Moons
-YkO 84 84  Sta                                              ; Fixed stars

-YkA 1 5    Yel Blu Red Gre Cya          ; Major aspect colors
-YkA 6 11   Mag Mag Mai Mai DkC DkC      ; Minor aspect colors
-YkA 12 18  DkC Mar Pur Pur Mar Mar Pur  ; Obscure aspect colors

-YkC        Red Yel Gre Blu                      ; Element colors
-Yk7 1 7    Red Ind For Yel Amb Ros Vio          ; Ray colors
-Yk0 1 7    Red Mai Yel Gre Cya Blu Pur          ; Rainbow colors
-Yk  0 8    Bla Whi LtG Gra Mar DkG DkC DkB Mag  ; Main colors


; OBJECT CUSTOMIZATION:

; [No objects are different from defaults]


; STAR CUSTOMIZATION:

; [No star objects are different from defaults]


; GRAPHICS DEFAULTS:

_XJ              ; Indian type wheels ["_XJ" is Western, "=XJ" is Indian  ]
=Xm              ; Color charts       ["=Xm" is color, "_Xm" is monochrome]
_Xr              ; Reverse background ["_Xr" is black, "=Xr" is white     ]
:Xw 1375 1375      ; Default X and Y resolution (not including sidebar)
:Xs 200          ; Character scale     [100-400]
:XS 150          ; Graphics text scale [100-400]
:XI0 25 1        ; Transparency % and background orientation [0-100, -1 to 1 ]
=XQ              ; Square charts ["=XQ" forces square, "_XQ" allows rectangle]
_XQ0             ; Autoscale     ["=XQ0" autoscales glyphs, "_XQ0" doesn't   ]
=Xu              ; Chart border  ["=Xu" shows border, "_Xu" doesn't show     ]
_Xx              ; Thicker lines ["=Xx" is thicker, "_Xx" is thinner         ]
_Xx0             ; Antialiasing  ["=Xx0" is antialiased lines, "_Xx0" is not ]
_XA              ; Glyphed lines ["=XA" glyphs on aspect lines, "_XA" doesn't]
_XL              ; Show cities   ["=XL" shows them in charts, "_XL" doesn't  ]
=Xv0             ; Show sidebar  ["=Xv0" shows on right edge, "_Xv0" doesn't ]
:Xv 1            ; Wheel fill    ["0" none, "1" standard, "2" rainbow, etc.  ]
:Xkv Aut         ; Wheel corners decoration color
:Xbw             ; Bitmap file type   ["Xbw" is Windows .bmp, "Xbp" is .png  ]
:YXG 211111      ; Glyphs for [Capricorn, Uranus, Pluto, Lilith, Vertex, Eris]
:YXg 0           ; Aspect grid cells  ["0" for autodetect  ]
:YXS 0.0         ; Orbit radius in AU ["0.0" for autodetect]
:YXj 0           ; Orbit trail count
:YX7 600         ; Esoteric Ray column influence width
:YXx 0           ; Line thickness adjustment for vector formats
:YXf #000000     ; Fonts to use [text, signs, houses, planets, aspects, naks.]
:YXp -1           ; PostScript paper orientation ["-1" portrait, "1" landscape]
:YXp0 54.84cm 70.97cm ; PostScript paper X and Y sizes

=X               ; Graphics chart display ["_X" is text, "=X" is graphics]


; MACROS:

-M0 37 "_sr -RO -1 -Y1 Ear Ear -YXt '' ~d '' ~O '' ~C '' ~A '' ~q2 '' ~Q1 '' ~Q2 '' ~Xt '' -M30 '' '' '' '' ~1 '=a 0'"
-M0 38 "_R Nor -Y1 Nor PluC ~C '=y ObjLon Add O_Asc Dec @x'"
-M0 39 "~O 'If Lt Lat 0.0 =w Add @w 180.0' ~C 'If Lt Lat 0.0 =y Add @y 180.0' -YXt '\nSpecial: Southern Zodiac'"
-M0 40 "=gp _g =sr0 =b0 _R Vul -YR0 1 1 -YR1 0 1 -YD Vul OOB ~O 'If Equ @v O_Vul Do =w 0.0 =x Oblique' ~d 'And Neq @u O_Vul Or Equ @w O_Vul Equ @v -5' =dm _X"
-M0 41 "-R0 Sun Moo Mer Ven Mar Jup Sat -c Whole -YJ Mar Ari Sco -YJ Jup Sag Pis -YJ Sat Cap Aqu -YJ Ura 0 0 -YJ Nep 0 0 -YJ Plu 0 0"
-M0 42 "~2 13 '-Ao Con 360 -Ao Opp 360' ~2 14 '-Ao Con ~@a -Ao Opp ~@b' -A 2 -YXa 0 ~Q1 '=a AspOrb A_Con =b AspOrb A_Opp Switch2 13' ~Q2 'Switch2 14' -~A '=c Neq _r 0 If And Equ @w A_Con Gt Abs Add LonDiff ObjLonN Mul @c 1 @v 90 LonDiff ObjLonN Mul @c 2 @x 90 @a =z -1 If And Equ @w A_Opp Gt Abs Add LonDiff ObjLonN Mul @c 1 @v 0 LonDiff ObjLonN Mul @c 2 @x 0 @b =z -1'"
-M0 43 "-i2 __1 -r2 ~1 '=a Not @a =b ?: @a 180 0 =c Sub 2 @a =d AspCol @c' ~20 1 ';Antiscia;Contra-Antiscia' -M20 '' '-x -1 :Ys ~@b' '-x 1 :Ys 0' -YXt '\n\4Special: \c'"
-M0 44 "-r2 -c Whole ~q2 'If Equ Context 1 =m JulianT If Equ Context 2 =n JulianT' ~Xt '=a Inc Mod Int Div Sub @n @m 365.24219 12 =b ObjCol Add O_Asc Dec @a' -YXt '\n\2Profection: House #\A' =X"
-M0 45 "-r2 ~20 1 ';Sun;Ven;Mer;Moo;Sat;Jup;Mar;Nor;Sou' ~3 1 '10 8 13 9 11 12 7 3 2' ~3 27 '1 4 3 2 7 6 5 16 17' ~q2 'If Equ Context 1 =m JulianT If Equ Context 2 =n JulianT' ~Xt '=o Div Sub @n @m 365.24219 =p ?: Lt LonDiff ObjLon1 O_Asc ObjLon1 O_Sun 0 1 4 While Gt @o Var @p Do =o Sub @o Var @p =p Inc Mod @p 9 =q ?: Gt @p 7 @p Inc Mod Add Int Mul Div @o Var @p 7.0 Dec @p 7 = 0 ObjCol Var Add 26 @p' -YXt '\n\0Firdaria: \p / \q' =X"
-M0 46 "_R For -r2 ~20 1 ';Ari;Tau;Gem;Can;Leo;Vir;Lib;Sco;Sag;Cap;Aqu;Pis' ~3 1 '15 8 20 25 19 20 8 15 12 27 30 12' ~q2 'If Equ Context 1 =m JulianT If Equ Context 2 =n JulianT' ~M 1 '=q Var @y While Gte @o Mul Var Var @y @x Do2 =o Sub @o Mul Var Var @y @x = @y ?: Lt Var @y 12 Inc Var @y 1 If Equ Var @y @q = @y Inc Mod Add Dec Var @y 6 12' ~Xt '=o Sub @n @m =s LonSign ObjLon1 O_For =x 360 =y %s Macro 1 =t @s =x 30 =y %t Macro 1 =u @t =x 2.5 =y %u Macro 1 =v @u =x Div @x 12 =y %v Macro 1 For %z 0 3 = Add 27 @z ObjCol Add O_Asc Dec Var Add %s @z' -YXt '\n\27Releasing L1: \s\n\28Releasing L2: \t\n\29Releasing L3: \u\n\30Releasing L4: \v' =X"
-M0 47 "=Xr -Xv 6 :XI0 100 1 -YkO Ear Eas Ray Ray Ray Ray Ray Ray Ray Ray Ray Ray Ray LtG LtG LtG LtG LtG LtG LtG LtG LtG LtG LtG -YkO Vul Vul Ray -YkO Hyg Orc LtG LtG Ray Ray Ray Ray Ray Ray Ray :YXG 113221"
-M0 48 "-M30 '-Y10 Ear Asc' '-Y10 Asc Asc' '-Y10 Sun Asc' '-Y1 Ear Ear' -i2 __1 -i3 __1 -r3 -c Whole _R Vul -YXt '\nSpecial: Triple Sun'"


; WINDOWS MENUS:

-WM 37 "&Clear Macro Settings"
-WM 38 "&Draconic Zodiac"
-WM 39 "&Southern Zodiac"
-WM 40 "&Out of Bounds"
-WM 41 "T&raditional Planets"
-WM 42 "&Antiscia Aspects"
-WM 43 "Antiscia &Wheel"
-WM 44 "Annual &Profection"
-WM 45 "&Firdaria Periods"
-WM 46 "&Zodiacal Releasing"
-WM 47 "&Esoteric Coloring"
-WM 48 "&Triple Sun"
-WM0 3 "Sample &Macros"


; WINDOWS DEFAULTS:

-WN 1000 ; Animation update delay in milliseconds
-Wx 6    ; Antialiasing line detail level ["1" is simplest, "12" is nicest   ]
_Wn      ; Buffer redraws                 ["=Wn" buffers, "_Wn" on screen    ]
=Wh      ; Hourglass cursor on redraw     ["=Wh" has hourglass, "_Wh" doesn't]
_Wt      ; Don't show popup messages      ["=Wt" doesn't show, "_Wt" shows   ]
=Wb      ; Export bitmaps based on window ["=Wb" from window, "_Wb" created  ]

; astrolog.as
