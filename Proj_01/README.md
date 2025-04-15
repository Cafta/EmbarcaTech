## EMBARCATECH - FASE 2 ##
### *Projeto no. 1*

---

### *Descrição:*

Faça um programa, em linguagem C, que implemente um contador decrescente controlado por interrupção, com o seguinte comportamento:

#### *Toda vez que o Botão A (GPIO5) for pressionado:*
* O contador decrescente reinicia em 9 e o valor da contagem é mostrado no display OLED.
* O sistema entra em modo de contagem regressiva ativa, decrementando o contador de 1 em 1 a cada segundo até chegar em zero.
* Durante essa contagem (ou seja, de 9 até 0), o programa deve registrar quantas vezes o Botão B (GPIO6) foi pressionado. O valor deste registro de eventos de botão pressionado também deve ser mostrado no display OLED.
* Quando o contador atingir zero, o sistema congela e ignora temporariamente os cliques no Botão B (eles não devem ser acumulados fora do intervalo ativo).

#### *O sistema permanece parado após a contagem, exibindo:*
* O valor 0 no contador
* A quantidade final de cliques no Botão B registrados durante o período de 9 segundo (contagem regressiva)

#### *Somente ao pressionar novamente o Botão A, o processo todo se reinicia:*
* O contador volta para 9
* O número de cliques do Botão B é zerado
* A contagem recomeça do início

### *Techniques used in this project:*  
* Button interrupts, Timer, and Alarm  
* 5x5 LED matrix  
* OLED display interface  

### *Material Utilizado:*
Foi utilizada apenas a placa ***BitDogLab v.6.3*** idealizada pelo Prof. Fabiano Fruet (UNICAMP).

---

![Imagem da placa](../pics/BitDogLab_v6.3.jpg)
