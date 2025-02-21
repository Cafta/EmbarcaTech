/*  EMBARCATECH - 2024/25
    UNIDADE 7   PROJETO FINAL DA PRIMEIRA FASE
    ALUNO: CARLOS FERNANDO MATTOS DO AMARAL
    DESCRIÇÃO:
  Montagem das páginas e mensagens para aparecerem no OLED
*/
const char PG_INITIAL[8][22] = {  // 8 linhas com 21 caracteres + 1 para `\0`
    "                     ",
    "     RCP HELPER      ",
    "                     ",
    "    Click em 'A'     ",
    "   quando iniciar    ",
    "       a RCP         ",
    "                     ",
    "                     "
  };  

const char PG_DEFAULT[8][22] = {  // 8 linhas com 21 caracteres + 1 para `\0`
    "00h00m        FCT:  0",
    "                     ",
    "                     ",
    "                     ",
    "                     ",
    "                     ",
    "                     ",
    "                     "
};  

const char MSG_INITIAL[6][11] = {
    "  CHECAR  ",
    "  RITIMO  ",
    " aperte A ",
    "ao iniciar",
    " massagem ",
    "          "
};

const char MSG_MASSAGEM[6][11] = {
    "Conduta:  ",
    "          ",
    " INICIAR  ",
    " MASSAGEM ",
    "          ",
    "          "
};

const char MSG_MASSAGEANDO[6][11] = {
    "          ",
    "REALIZANDO",
    " MASSAGEM ",
    "          ",
    "          ",
    "          "
};

const char MSG_RCE[6][11] = {
    "   RCE    ",
    "          ",
    " INICIAR  ",
    " TERAPIA  ",
    " POS PCR  ",
    "          "
};

const char MSG_CHECAR_PULSO[6][11] = {
    "Conduta:  ",
    "          ",
    "  CHECAR  ",
    "  PULSO   ",
    "          ",
    "          "
};

const char MSG_CHECAR_RITMO[6][11] = {
    "Conduta:  ",
    "  CHECAR  ",
    "  RITMO   ",
    " CARDIACO ",
    "          ",
    "          "
};

const char MSG_PREPARA_TROCA[6][11] = {
    "Conduta:  ",
    "          ",
    " PREPARAR ",
    "  TROCA   ",
    "MASSAGISTA",
    "          "
};

const char MSG_TROCAR_MASSAGISTA[6][11] = {
    "Conduta:  ",
    "          ",
    "  TROCAR  ",
    "MASSAGISTA",
    "          ",
    "          "
};

const char MSG_DESFIBRILAR[6][11] = {
    "Conduta:  ",
    "  APLICAR ",
    "  CHOQUE  ",
    " (DESFIB) ",
    "          ",
    "          "
};

const char MSG_CARDIOVERTER[6][11] = {
    "Conduta:  ",
    "  CARDIO- ",
    "  VERSAO  ",
    "   SINC   ",
    "          ",
    "          "
};

const char MSG_EPINEFRINA[6][11] = {
    "Conduta:  ",
    "          ",
    "EPINEFRINA",
    "  1mg EV  ",
    "          ",
    "          "
};

const char MSG_AMIODARONA300[6][11] = {
    "Conduta:  ",
    "          ",
    "AMIODARONA",
    " 300mg EV ",
    "          ",
    "          "
};

const char MSG_AMIODARONA150[6][11] = {
    "Conduta:  ",
    "          ",
    "AMIODARONA",
    " 150mg EV ",
    "          ",
    "          "
};
