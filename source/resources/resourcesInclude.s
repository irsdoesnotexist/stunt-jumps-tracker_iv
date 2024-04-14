.section .rodata
    .global vertSpv
    .type vertSpv, @object
    .type vertSpv_size, @object
    .global vertSpv_size

    .global fragSpv
    .global fragSpv_size
    .type fragSpv, @object
    .type fragSpv_size, @object
    .ballign 4


vertSpv:
    .incbin "vert.spv"
vertSpv_end:
    .ballign 4
vertSpv_size:
    .int vertSpv_end - vertSpv
    .ballign 4

fragSpv:
    .incbin "frag.spv"
fragSpv_end:
    .ballign 4
fragSpv_size:
    .int fragSpv_end - fragSpv
    .ballign 4