; SCRIPT FOR "medieval total war 2"

; Memory For New Code
new		injectedCode	8192

; New Code
asm <injectedCode>
{

; this make tax income ZERO when tax income is set to low
TaxIncome:						mov ecx, esi
								sub ecx, 000009D0
								mov ecx, [ecx]
								cmp ecx, 00000000
TaxIncome0:						jg %TaxIncome1%
								mov eax, 00000000
TaxIncome1:						mov [esi+00000090], eax
								jmp 005E8BA1

; this changes how much taxes effect birth rate
x4TaxBirths:					shl eax, 02
								mov [esi+14], eax
								mov [esp+50], esp
								jmp 005E7B2F

; this changes how much taxes effect death rate
x4TaxDeaths:					shl eax, 02
								pop edi
								mov [esi+2C], eax
								pop esi
								jmp 005E7C18

; this changes how much taxes effect public order
x4TaxOrder:						mov ecx, esp
								shl eax, 01
								mov [esi+44], eax
								jmp 005E8216


; this disables the distance to capital effect
DistCap:						mov ecx, esp
								mov eax, 00000000
								mov [esi+68], eax
								jmp 005E86C0

; this reduces the currption
Currupt:						shl eax, 01
								mov [esi+000000C4],eax
								jmp 005E8F16

; this changes the religous unrest
RelUnRest:						shl eax, 02
								mov [esi+00000080],eax
								jmp 005E87B9

; this increases the effect of farming
Farming:						shr eax, 02
								mov [esi+0000008C],eax
								jmp 005E8B6F

; this increases the effect of mining
Mining:							shl eax, 01
								mov [esi+00000094],eax
								jmp 005E8BCA

;Trade:							shl eax, 01
;								mov [esi+00000098],eax
;								jmp 005E8BF9

Law:							mov ecx, esp
								shl eax, 02
								mov [esi+38],eax
								jmp 005E819B

Plague:							add esp, 10
								mov [esi+28], 00000000
								jmp 005E7BED

Squalor:						mov ecx, esp
								mov [esi+24], 00000000
								jmp 005E7BCF

; the effect of Squalor on public order should be ZERO. This is not COVID
SqualorPO:						mov [ebp+00], 00000000
								mov [esp+6C], esp
								jmp 005E869B

; MORE DEATHS IN AUTO RESOLVED BATTLES
AutoResolve:					sub eax,[esi+00000108]
								sub eax,[esi+00000108]
								sub eax,[esi+00000108]
								cmp eax, 00000000
								jge %AutoResolve2%
								mov eax, 00000000
AutoResolve2:					jmp 0061B858
}



; Disable tax (income)
asm 005E8B9B
{
								jmp %TaxIncome%
								nop
}

; tax effect on births (births/deaths)
asm 005E7B28
{
								jmp %x4TaxBirths%
								nop
								nop
}
asm 005E7C13
{
								jmp %x4TaxDeaths%
}

; Double tax (order/unrest)
asm 005E8211
{
								jmp %x4TaxOrder%
}


; Dist
asm 005E86BB
{
								jmp %DistCap%
}

; Curruption
asm 005E8F10
{
								jmp %Currupt%
								nop
}

; ReliUnRest
asm 005E87B3
{
								jmp %RelUnRest%
								nop

}

; Farm
asm 005E8B69
{
								jmp %Farming%
								nop

}

;Mining
asm 005E8BC4
{
								jmp %Mining%
								nop
}

;trade
;asm 005E8BF3
;{
;								jmp %Trade%
;								nop
;}

;Law
asm 005E8196
{
								jmp %Law%
}

;disable plague
asm 005E7BE7
{
								jmp %Plague%
								nop
}

;disable Squalor
asm 005E7BCA
{
								jmp %Squalor%
}

;disable Squalor Public Order
asm 005E8694
{
								jmp %SqualorPO%
								nop
								nop
}

;Harsher Auto Resolve For Ships (May work on armies too, I did not test)
asm 0061B852
{
								jmp %AutoResolve%
								nop
}
