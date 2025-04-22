.intel_syntax noprefix
.global grayscaleASM

.text

// grayscale conversion in assembler
grayscaleASM: 

// calculates (a + b + c) in xmm3
movss xmm3, xmm2
addss xmm3, xmm1
addss xmm3, xmm0

// rax is the counter for the input array
xor rax, rax

// rcx is the counter for the output array
xor rcx, rcx

xor r8, r8
xor r9, r9
xor r10, r10

// iterates over every pixel
loop:

cmp rcx, rdx
jge end

// loads red
mov r8b, byte ptr [rdi+rax]

//loads green
mov r9b, byte ptr [rdi+rax+1]

/loads blue
mov r10b, byte ptr [rdi+rax+2]

// converts them to 32-Bit floats
cvtsi2ss xmm4, r8d
cvtsi2ss xmm5, r9d
cvtsi2ss xmm6, r10d

mulss xmm4, xmm0
mulss xmm5, xmm1
mulss xmm6, xmm2

addss xmm4, xmm5
addss xmm4, xmm6

divss xmm4, xmm3

// converts the result from 32-Bit floats to 8-Bit Integer
cvtss2si r8d, xmm4

mov byte ptr[rsi+rcx], r8b

// increase counters
inc rcx
add rax, 3

jmp loop

end:
