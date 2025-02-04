REQUIRE CASE lib/ext/case.f
REQUIRE !CSP ~mak/lib/csr.f

: DEFER CREATE ['] NOOP , DOES> @ EXECUTE ;
: DEFER@ ' >BODY STATE @ IF POSTPONE LITERAL POSTPONE @ ELSE @ THEN ; IMMEDIATE
: IS '  >BODY STATE @ IF POSTPONE LITERAL POSTPONE ! ELSE ! THEN ; IMMEDIATE

: +TO ' >BODY STATE @ IF POSTPONE LITERAL POSTPONE +! ELSE +! THEN ; IMMEDIATE

: REVEAL SMUDGE ;
: COMPILE ' POSTPONE LITERAL POSTPONE COMPILE, ; IMMEDIATE
: (;CODE) R> LATEST 5 - ! ;

VECT EXIT-ASSEMBLER

VOCABULARY ASSEMBLER

S" lib/asm/486asm.f" INCLUDED

( FORTH HEADER CREATION WORDS )
ALSO ASSEMBLER ALSO ASM-HIDDEN
IN-HIDDEN
: _CODE ( START A NATIVE CODE DEFINITION )
        CREATE CFL NEGATE ALLOT HIDE !CSP INIT-ASM ;

: _;CODE ( CREATE THE [;CODE] PART OF A LOW LEVEL DEFINING WORD )
        ?CSP !CSP COMPILE (;CODE) POSTPONE [ INIT-ASM ;

IN-FORTH
' _CODE IS CODE
' _;CODE IS ;CODE

: FCALL A; [COMPILE] ' COMPILE, ;

ONLY FORTH DEFINITIONS

ALSO FORTH IMMEDIATE PREVIOUS


S" lib/asm/asmmac.f" INCLUDED
