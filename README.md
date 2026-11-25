# Arena Survival - ED1

Jogo 2D de sobrevivência em arena feito em C com [raylib](https://www.raylib.com/), desenvolvido para a disciplina de Estrutura de Dados I.

Sobreviva a 5 ondas de inimigos (orcs, arqueiros e um chefe na última onda) usando espada, esquiva e poções.

## Controles

| Tecla | Ação |
|---|---|
| WASD / Setas | Mover |
| Espaço | Atacar com a espada |
| Shift | Esquivar (invulnerável durante o movimento) |
| Q | Usar a última poção coletada |
| ESC ou P | Pausar |
| Enter | Confirmar nos menus |

## Como compilar (Windows, raylib + w64devkit)

Abra a pasta do repositório no VS Code e rode a tarefa **Compilar Raylib** (`Ctrl+Shift+B`). O executável `jogo.exe` é gerado em `ProjetoJogo/Codigo`. Execute-o de dentro dessa pasta, para que a pasta `assets/` seja encontrada.

Ajuste os caminhos `C:/raylib/...` do `tasks.json` se a raylib estiver instalada em outro lugar.

## Estruturas de dados usadas

| Estrutura | Arquivos | Para que serve |
|---|---|---|
| Lista encadeada | `lista_inimigos` | Inimigos vivos na arena (inserção, remoção dos mortos no meio da lista) |
| Lista encadeada | `lista_itens` | Poções caídas no chão (expiram com o tempo; coletadas ao passar por cima) |
| Lista encadeada | `lista_projeteis` | Flechas em voo (removidas ao acertar ou sair da arena) |
| Fila | `fila_ondas` | Ondas na ordem em que aparecem (`FilaEnfileirar`, `FilaDesenfileirar`, `FilaEspiar`) |
| Pilha (vetor) | `pilha_pocoes` | Mochila de até 5 poções: a última coletada é a primeira a ser usada |
| Pilha (encadeada) | `pilha_telas` | Telas do jogo: Menu → Jogo → Pausa → Controles. ESC/Enter desempilha e volta à tela anterior |

## Módulos

| Módulo | Responsabilidade |
|---|---|
| `main` | Janela e laço principal |
| `entrada` | Lê o teclado e monta uma struct `Entrada` (facilita testar sem teclado) |
| `telas` | Menu, pausa e controles, controlados pela pilha de telas |
| `partida` | Regras do jogo: atualização, colisões, drops, ondas e desenho da partida |
| `player` | Movimento, vida, esquiva com rastro e cura |
| `arma` | Espada: animação de golpe e momento do impacto |
| `inimigo` | Orc, arqueiro e chefe: máquina de estados da IA, telégrafo de ataque, animações |
| `ondas` | Composição das ondas e geração dos inimigos |
| `arena` | Fundo e limites da arena |
| `ui` | HUD, menus e telas finais |
| `config.h` | Constantes globais (tamanho da janela) |

## Inimigos

- **Orc**: persegue e, ao chegar perto, prepara o golpe. Um círculo vermelho no chão cresce mostrando a área; saia dela (ou esquive) antes de fechar. Acertar o orc enquanto ele prepara interrompe o ataque.
- **Arqueiro**: mantém distância e mira. Uma linha vermelha mostra a direção; ela trava um instante antes do disparo, então mova-se depois que travar.
- **Chefe** (onda 5): grande, resistente, não é atordoado pelos golpes e dá um golpe em área com aviso longo. Derrubado, solta duas poções grandes.

## Estrutura do repositório

```
ProjetoJogo/Codigo/   código-fonte (.c/.h) e assets/
.vscode/tasks.json    tarefa de compilação
```
