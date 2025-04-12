/*    EMBARCATECH - Abr/2025
           -- 2a fase --
    autor: Carlos Amaral (CAFÉ)

   Esquema dos números na matrix
   de leds 5x5 da placa BitDogLab
   v.6.3
*/
#define numero_de_leds 25
#define numero_de_numeros 10
const uint32_t RED = 0x000600, BLUE = 0x000006, GREEN = 0x060000, WHITE = 0x060606, BLACK = 0x000000;

const uint32_t numero[numero_de_numeros][numero_de_leds] = {
    {   // 0 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Terceira linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    },
    {   // 1 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, BLACK, WHITE, BLACK, BLACK,  // Quinta linha
      BLACK, BLACK, WHITE, BLACK, BLACK, // Quarta linha
      BLACK, BLACK, WHITE, BLACK, BLACK, // Terceira linha
      BLACK, WHITE, WHITE, BLACK, BLACK, // Segunda linha
      BLACK, BLACK, WHITE, BLACK, BLACK, // Primeira linha
    },
    {   // 2 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, WHITE, BLACK, BLACK, BLACK, // Quarta linha
      BLACK, BLACK, WHITE, BLACK, BLACK, // Terceira linha
      BLACK, BLACK, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    },
    {   // 3 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, BLACK, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Terceira linha
      BLACK, BLACK, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    },
    {   // 4 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, BLACK, BLACK, BLACK,  // Quinta linha
      BLACK, BLACK, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Terceira linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Primeira linha
    },
    {   // 5 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, BLACK, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Terceira linha
      BLACK, WHITE, BLACK, BLACK, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    },
    {   // 6 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Terceira linha
      BLACK, WHITE, BLACK, BLACK, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    },
    {   // 7 - CUIDADO! as linhas são "de cabeça pra baixo" e as linhas "vão e voltam"
      BLACK, WHITE, BLACK, BLACK, BLACK,  // Quinta linha - essa linha inverte verticalmente também
      BLACK, BLACK, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, BLACK, BLACK, BLACK, // Terceira linha - Essa linha também
      BLACK, BLACK, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha - E essa também
    },
    {   // 8 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Terceira linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    },
    {   // 9 - CUIDADO! as linhas são "de cabeça pra baixo"
      BLACK, WHITE, WHITE, WHITE, BLACK,  // Quinta linha
      BLACK, BLACK, BLACK, WHITE, BLACK, // Quarta linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Terceira linha
      BLACK, WHITE, BLACK, WHITE, BLACK, // Segunda linha
      BLACK, WHITE, WHITE, WHITE, BLACK, // Primeira linha
    }
};