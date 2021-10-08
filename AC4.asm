; loader AC4BFSP.exe AC4.asm

; hacks Assassin's creed 4: black flag

; allocate memory (in bytes)
new		oneHitKill		64

; put new code in memory
asm <oneHitKill>
{
			movsx eax,word ptr [ecx+60]			; test if is player
			cmp eax, 01							; if 1, it's the player
			je %isPlyr%
			mov eax, 00000001
			ret
isPlyr:		movsx eax,word ptr [ecx+5C]
			ret
}

; reroute the code
asm 00FB4A50 
{
			jmp <oneHitKill>
}
