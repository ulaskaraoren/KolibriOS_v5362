( �� ��࠭���:
  �⥪ ������ ��� 㭨���ᠫ쭮� �।�⢮ ��� ��।�� ��ࠬ��஢ � १���⮢ 
����� ���-᫮���� ����� ���ᯮਬ� �२����⢠. ����� � ⥬ ����� 
��।������ �� �ᯮ������ ��� �஬������� ���᫥��� � ࠧ��饭�� ���祭��,
����� � ��� �������. �� ��뢠�� ��।������ ��㤭��� ��� ����㯠 � 
⠪��� �����쭮�� ���祭��, ��᪮��� ��� ��������� �⭮�⥫쭮 ���設� �⥪� 
����ﭭ� �������.
  ��� ��饭�� ࠡ��� ����⥫쭮 ���९��� �� ������� ��ꥪ⠬� ����� 
��।������ ������� ����ﭭ� �����, �१ ����� � �����⢫��� ����� � 
���.
  ����騩�� � �몥 ��堭��� ���ᠭ�� ��६����� � ������ ��砥 �� ���室��, 
��᪮��� ᮧ���� �������� �����, ⮣�� ��� �ॡ���� ��������� ������� 
��ꥪ��, ���뢠� �� �⮬ ����������� ४��ᨢ��� �맮���. ���⠢������ ������ 
�蠥� ����祭�� � ࠡ��� �������⥫쭮�� �⥪�, �⫨筮�� �� �⥪� ������. 
������� ���祭�� ࠧ������� � �⮬ �⥪� �� �室� � ��।������ � 㡨����� 
�� ���� �� ��室�.
  �� �� �६� �ᯮ������ ��।������ �� ��������� �⭮�⥫쭮 ���設� �⥪� 
��⠥��� ����ﭭ�, �� �������� �࣠�������� �祭� ���⮩ ����� � ⠪�� 
���祭��.
  ���⥩�� �����ன�� ��� �몮� ����, ����� �������� ࠡ���� � ������묨
��६���묨, �룫廊� ⠪:)


100 ALLOT   HERE CONSTANT LP0 ( ������ ���.�����) 
VARIABLE LP  ( ������� ������� ���������� �����)
: INITLP ( ->)  LP0 LP ! ; INITLP
: LOC ( N:�������->) 1+ CELLS LP @ OVER - DUP LP ! ! ;
: UNLOC ( ->)  LP @ @ LP +! ;
: @@ ( N:����->) CREATE , DOES> ( PFA->A) @ CELLS LP @ + @ ;
: !! ( N:����->) CREATE , DOES> ( A,PFA->) @ CELLS LP @ + ! ;

1 @@ @1  2 @@ @2  3 @@ @3  4 @@ @4  5 @@ @5 ( � �.�.)

1 !! !1  2 !! !2  3 !! !3  4 !! !4  5 !! !5 ( � �.�.)
