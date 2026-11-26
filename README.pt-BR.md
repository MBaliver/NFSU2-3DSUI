# Need for Speed: Underground 2 para New 3DS — Fork de UI/UX

> 🌐 **Idioma / Language:** **[🇺🇸 English](README.md)** | **Português (Brasil)**

Um fork focado em UX/UI do [Octoblimp/NFSU2-3DS](https://github.com/Octoblimp/NFSU2-3DS) (baseado na versão `v1.0.1` do projeto original): um port homebrew nativo e não oficial da versão de PC de **Need for Speed: Underground 2** para **New Nintendo 3DS / New 3DS XL / New 2DS XL**.

Este é o próprio jogo de PC — não um remake. O código foi obtido por engenharia reversa do `SPEED2.EXE` e reimplementado em C (usando a versão de GameCube como referência de console). O jogo roda usando os arquivos convertidos da **sua própria cópia de PC**; **nenhum dado protegido por direitos autorais do jogo é distribuído** neste repositório ou nas releases.

Você tem o jogo de PC completo — mundo aberto em Bayview, modo Carreira, lojas, patrocinadores, capas de revista e DVD, mensagens SMS, mapa-múndi, todos os modos de corrida (Circuit, Sprint, Drift, Drag, Street X, URL, Outrun), Quick Race, customização completa, dinamômetro e tuning, cutscenes, EA TRAX, saves, códigos de trapaça e multiplayer Local Wireless para até 6 consoles — **aprimorado com uma interface touch redesenhada para o 3DS, suporte a 8 idiomas, opções de acessibilidade, direção por movimento e melhorias de qualidade de vida.**

> **Já joga o NFSU2-3DS original do Octoblimp?** Sua pasta `sdmc:/nfsu2` e seus saves de carreira são **100% compatíveis**. Basta instalar o `NFSU2-3DS.cia` deste fork pelo FBI — seus perfis e saves permanecem intactos!

---

## Índice

- [Novidades deste Fork](#novidades-deste-fork)
- [Requisitos](#requisitos)
- [Como Jogar (Instalação)](#como-jogar-instalação)
  - [Atualizando a partir do NFSU2-3DS original](#atualizando-a-partir-do-nfsu2-3ds-original)
  - [Instalação do Zero (Windows — Interface Gráfica)](#instalação-do-zero-windows--interface-gráfica)
  - [Instalação do Zero (Linux / macOS / Python CLI)](#instalação-do-zero-linux--macos--python-cli)
  - [Carros Regionais e Áudio](#carros-regionais-e-áudio)
- [Controles e Opções do Fork](#controles-e-opções-do-fork)
- [Local Wireless, Cheats e Mods](#local-wireless-cheats-e-mods)
- [Arquivo de Configuração (`nfsu2_config.txt`)](#arquivo-de-configuração-nfsu2_configtxt)
- [Regiões](#regiões)
- [Compilando a partir do Código-Fonte (Build)](#compilando-a-partir-do-código-fonte-build)
  - [Pré-requisitos](#pré-requisitos)
  - [Compilando no Linux](#compilando-no-linux)
  - [Compilando no Windows](#compilando-no-windows)
  - [Compilando as Ferramentas de PC (Opcional)](#compilando-as-ferramentas-de-pc-opcional)
  - [Estrutura do Repositório](#estrutura-do-repositório)
- [Reportando Bugs](#reportando-bugs)
- [Sobre, Créditos e Licença](#sobre-créditos-e-licença)

---

## Novidades deste Fork

Todas as melhorias abaixo ficam dentro do executável do 3DS, mantendo total compatibilidade com os saves originais e com os dados de jogo do upstream (veja [docs/bottom-screen-ui.md](docs/bottom-screen-ui.md) para os detalhes técnicos):

- **Kit de Interface Nativo para a Tela Inferior do 3DS (`U2-UI`)**
  - Cerca de 90 telas de menu e overlays redesenhadas para a tela touch 320×240 usando a identidade visual do próprio NFSU2, formas vetoriais com antialiasing (`uiaa`), áreas de toque mínimas de 32 px, fontes sempre legíveis (rótulos ≥ 12 px / texto ≥ 14–16 px), feedback sonoro ao tocar, barras de rolagem arrastáveis, diálogos longos com rolagem e seleção direta por toque.
  - Inclui hubs de ícones em grade, Seleção de Carro (Garagem, Concessionária, Quick Race, LAN, Customizar), listas de peças de carroceria e performance, paleta de pintura, gráficos do dinamômetro e tuning, EA Trax, salas LAN, Caixa de Entrada e leitor de SMS, Status da Carreira e diálogos de Ajuda/Engajamento.
- **HUD Touch Reformulado em Free Roam e Corridas**
  - Minimapa em tela cheia na tela inferior com zoom (`+`/`−`), arraste por toque e botão `RECENTER`.
  - **Rota GPS traçada nas ruas em tempo real** com **distância real pelo trajeto** (não em linha reta) e cores combinando com o destino escolhido.
  - Toque em qualquer marcador no mapa para ver detalhes e ativar **SET GPS** com um toque; toque no chip de dinheiro ou SMS para abrir direto o Status da Carreira ou a Caixa de Mensagens; pílula contextual de **Entrar / Desafiar**; botão opcional **Touch Camera**.
  - Interface de corrida unificada com chips flutuantes de posição/volta/tempo, bandeira quadriculada na linha de chegada e barra expansível de **Classificação (Standings)** via D-Pad Baixo ou toque.
- **Mapa-Múndi Renovado**
  - Mapa em tela cheia arrastável e com zoom, ícones consistentes com o minimapa do HUD, corredores da IA visíveis no free roam, toque direto nos marcadores e gaveta lateral de filtros com rolagem.
- **Tela Inicial, Carregamento, Pausa, Pós-Corrida e Replay Otimizados**
  - Cartão de perfil na tela inicial com **Resume Career / Continuar** (vai direto para o mundo aberto sem travar ou piscar no menu principal; um toque pula a introdução).
  - Carregamento mais rápido ao entrar no mundo (pré-carrega apenas os trechos próximos ao carro) com tela inferior informativa exibindo controles e dicas do jogo.
  - Menu de **Pausa** redesenhado na tela inferior (com mapa e resumo da carreira na tela superior), telas navegáveis de **Resultados e Estatísticas Pós-Corrida** e **Deck de Replay** touch completo com linha do tempo arrastável.
- **8 Idiomas + Página de Acessibilidade (*Options > Accessibility*)**
  - Jogue em **English, Français, Deutsch, Italiano, Español, Nederlands, Svenska ou Dansk** — traduzindo tanto os textos do jogo (`LANGUAGES/<Idioma>.bin`) quanto todas as strings e dicas próprias do port (incluindo correção para espaços inquebráveis em francês).
  - Modos **Texto Grande** (~1,25×) e **Alto Contraste**, salvos por perfil em um arquivo `port.txt` separado para manter seu save original intacto byte a byte.
- **Novos Controles e Opções para o 3DS**
  - **Direção por Movimento / Motion Steering** (*Options > Player*): incline o 3DS para dirigir usando o acelerômetro do console.
  - **Drag Shift: ZL/ZR** (*Options > Gameplay*): troque marchas em corridas de Drag usando `ZR` (subir) e `ZL` (reduzir).
  - **Touch Camera** (*Options > Gameplay*): botão touch opcional no canto superior direito da tela inferior para alternar a câmera.
  - **Câmera Livre no C-Stick** nas câmeras traseiras (além de hidráulica, câmera do dinamômetro e mover o mapa).
- **Suporte a Toolchain Moderna e Build no Linux**
  - Inclui script de compilação nativo para Linux (`tools/build_cia.sh`), detecção em tempo de execução da medição de texto do `citro2d` 1.6 / 1.7+ e compatibilidade com `libctru` 2.7.1.

---

## Requisitos

- Um **New Nintendo 3DS, New 3DS XL ou New 2DS XL** com Custom Firmware (**Luma3DS**) e **FBI**.
  - *Consoles Old 3DS / Old 3DS XL / Old 2DS não conseguem rodar o port* (pouca memória RAM e CPU insuficiente).
- **Firmware DSP** extraído para ter som:
  - Abra o menu **Rosalina** (`L + D-Pad Baixo + SELECT`) → **Miscellaneous options** → **Dump DSP firmware**.
  - Isso cria `sdmc:/3ds/dspfirm.cdc` (para `.3dsx`) ou `sdmc:/dspfirm.cdc` (para `.cia` — se só existir em `/3ds/dspfirm.cdc`, copie também para a raiz do SD em `sdmc:/dspfirm.cdc`).
- **Sua própria cópia de PC** de *Need for Speed: Underground 2* (a pasta onde fica o `SPEED2.EXE`).
  - Testado com a **versão americana (US) atualizada para v1.2**. Outras versões/patches não têm garantia de funcionamento.
- Cerca de **2,6 GB** livres no cartão SD.
- Um PC (Windows, Linux ou macOS) para converter os arquivos da sua cópia de PC uma única vez para o cartão SD.

---

## Como Jogar (Instalação)

### Atualizando a partir do NFSU2-3DS original

Se você já tem a pasta `sdmc:/nfsu2` funcionando do [Octoblimp/NFSU2-3DS](https://github.com/Octoblimp/NFSU2-3DS):

1. Baixe o arquivo **`NFSU2-3DS.cia`** mais recente na página de [**Releases**](https://github.com/MBaliver/NFSU2-3DSUI/releases) deste repositório.
2. Copie o `NFSU2-3DS.cia` para o cartão SD do 3DS e instale-o pelo **FBI** (ele substitui o aplicativo mantendo todos os seus dados e saves intactos).
3. *(Opcional — para outros idiomas)* Copie os arquivos de idioma desejados (`French.bin`, `German.bin`, `Italian.bin`, `Spanish.bin`, `Dutch.bin`, `Swedish.bin`, `Danish.bin`) da pasta `LANGUAGES/` do seu jogo de PC para `sdmc:/nfsu2/LANGUAGES/`, e escolha o idioma em **Options > Accessibility > Language**.
4. Abra o **Need for Speed: Underground 2** pelo Menu HOME!

### Instalação do Zero (Windows — Interface Gráfica)

1. **Converta os arquivos da sua cópia de PC para o cartão SD:**
   - **Recomendado:** Baixe o `NFSU2-3DS-Tools.zip` na [release v1.0.1 do projeto original](https://github.com/Octoblimp/NFSU2-3DS/releases) (já vem com `ffmpeg.exe`, `vgmstream`, `dspenc` e `tex3ds` prontos, sem precisar instalar nada), **ou** clone este repositório com **Git LFS** instalado e abra [`PC Apps/NFSU2-3DS Tools`](PC%20Apps/NFSU2-3DS%20Tools).
     > **Atenção ao botão "Download ZIP" do GitHub:** o `ffmpeg.exe` é armazenado via Git LFS, então o ZIP do código-fonte contém apenas um arquivo-ponteiro pequeno. Use o ZIP das ferramentas, clone com Git LFS ou tenha o `ffmpeg` (com `libx264`) no `PATH`.
   - Abra o **NFSU2-3DS Data Setup** (`NFSU2-3DS Data Setup.exe`).
   - Selecione a pasta da sua instalação de PC do NFSU2 (onde está o `SPEED2.EXE`).
   - Como pasta de destino, escolha uma pasta chamada `nfsu2` na **raiz do cartão SD** (por exemplo, `E:\nfsu2`).
   - Clique em **Set up / update**. A primeira execução demora um pouco; execuções futuras só processam o que estiver faltando ou desatualizado.
   - *(Opcional — outros idiomas)* Se você usou o ZIP das ferramentas do upstream e quer outros idiomas além de inglês, copie os arquivos `.bin` desejados (`Spanish.bin`, `French.bin`, etc.) da pasta `LANGUAGES\` do jogo de PC para `<SD>:\nfsu2\LANGUAGES\`. (Usando o `stage.py` deste fork isso já é feito automaticamente.)
   - *Alternativa por linha de comando no Windows:*
     ```cmd
     nfsu2-3ds.exe setup --pc "C:\Caminho\Para\NFSU2" --gamedir E:\nfsu2
     nfsu2-3ds.exe setup --pc "C:\Caminho\Para\NFSU2" --gamedir E:\nfsu2 --list
     ```
2. **Instale o jogo no New 3DS:**
   - Baixe o **`NFSU2-3DS.cia`** na página de [**Releases**](https://github.com/MBaliver/NFSU2-3DSUI/releases) deste repositório (ou [compile você mesmo](#compilando-a-partir-do-código-fonte-build)).
   - Copie o `NFSU2-3DS.cia` para o cartão SD, instale usando o **FBI** e inicie pelo **Menu HOME**.

> **CIA vs `.3dsx`:** A versão **`.cia`** é fortemente recomendada e testada em hardware real — apenas o CIA recebe o modo de memória estendida de 178 MB do New 3DS. Na primeira inicialização, o jogo cria automaticamente o `sdmc:/nfsu2_config.txt` apontando para `/nfsu2`. O `.3dsx` roda pelo Homebrew Launcher usando `/3ds/nfsu2`.

> **Saves e Diagnóstico de Arquivos:** Seus perfis e saves ficam em `sdmc:/nfsu2/SAVE/` e `sdmc:/nfsu2/profiles/`. O Data Setup nunca apaga nem modifica seus saves. Se faltar algum arquivo (config, pasta `GameDir` ou `dspfirm.cdc`), o jogo exibe uma tela de aviso mostrando exatamente qual caminho faltou, em vez de travar em tela preta.

### Instalação do Zero (Linux / macOS / Python CLI)

Você pode converter os arquivos do jogo em qualquer sistema usando os scripts Python em `tools/`:

1. Instale o **Python 3** e as bibliotecas necessárias:
   ```bash
   pip install numpy Pillow lz4 pefile unicorn capstone
   ```
2. Tenha no seu `PATH`:
   - `ffmpeg` (com suporte a `libx264`)
   - `vgmstream-cli`
   - `dspenc` (o código-fonte está em `PC Apps/NFSU2-3DS Tools/bin/dspenc.c`)
   - `tex3ds` do devkitPro (`sudo dkp-pacman -S 3dstools` no Linux/macOS)
3. Execute o script de preparação (que também copia automaticamente todos os idiomas disponíveis!):
   ```bash
   python3 tools/stage.py --pc "/caminho/para/NFSU2" --gamedir "/caminho/para/sd/nfsu2"
   ```
   Ou abra a interface gráfica com `python3 tools/nfsu2_setup.pyw`.

### Carros Regionais e Áudio

- **Peugeot 106 e Vauxhall/Opel Corsa:** Esses dois carros só existem na versão europeia (UK/EU) de PC (a versão US tem `CIVIC` e `RSX`). O Data Setup empacota os carros presentes na sua pasta `CARS\` e ignora os ausentes.
  - Para ter o 106 e o Corsa numa cópia americana, copie as pastas `CARS\PEUGOT106` e `CARS\CORSA` de uma cópia europeia para a pasta `CARS` do seu PC antes de rodar o Data Setup (ou rode novamente depois) e ajuste [`Region=`](#regiões) (`EU`, `ALL`, etc.). Sem essas pastas, mantenha `Region=US`.
  - O inverso vale para cópias UK/EU sem `CARS\CIVIC` e `CARS\RSX`: use `Region=EU` ou copie essas duas pastas de uma cópia US.
- **Música e Vozes em `SDATA\sdat.viv`:** Algumas instalações guardam músicas e falas dentro do arquivo `SDATA\sdat.viv` em vez das pastas `pfdata` e `speech`. O Data Setup extrai isso automaticamente para um cache temporário sem alterar a pasta do jogo no PC.

---

## Controles e Opções do Fork

### Dirigindo

| Controle | Ação |
| --- | --- |
| **Circle Pad** *(ou inclinação do 3DS com Motion Steering ativo)* | Esterçar |
| **A** | Acelerar |
| **B** | Frear / Ré |
| **R** | Nitro |
| **L** | Freio de mão |
| **X** | Subir marcha |
| **Y** | Reduzir marcha |
| **ZR** | Mudar câmera *(ou subir marcha em Drag com **Drag Shift: ZL/ZR** ativo)* |
| **ZL** | Resetar carro *(ou reduzir marcha em Drag com **Drag Shift: ZL/ZR** ativo)* |
| **C-Stick** | Câmera livre (visões traseiras) / Hidráulica / Mover o mapa |
| **SELECT** | Olhar para trás |
| **D-Pad Cima** | Entrar em loja / Desafiar oponente |
| **D-Pad Baixo** | Mapa-Múndi (free roam) / Alternar Classificação (em corridas) |
| **D-Pad Esquerda** | Estatísticas da Carreira |
| **D-Pad Direita** | Mensagens SMS |
| **START** | Pausar |

Todas as ações de direção (exceto esterçar e pausar) podem ser remapeadas por perfil em **Options > Controls** (**Default Settings** restaura o padrão e **Delete All Controls** limpa todos os botões).

### Menus e Tela Touch

- Navegue com o **D-Pad** ou **Circle Pad**, confirme com **A**, volte com **B** e role listas ou diálogos longos arrastando o dedo ou usando **Cima/Baixo**.
- Todas as telas redesenhadas na tela inferior funcionam por **toque direto**, incluindo o minimapa no mundo aberto (arraste para mover, `+`/`−` para zoom, toque nos ícones para traçar GPS), Mapa-Múndi, Caixa de SMS, Pausa, Pós-Corrida e Deck de Replay.

### Opções Extras Adicionadas por este Fork

- **Options > Accessibility:** `Language` (8 idiomas), `Large Text` (texto grande), `High Contrast` (alto contraste).
- **Options > Gameplay:** `GPS Color`, `Drag Shift: ZL/ZR`, `Touch Camera`, `Car Damage`.
- **Options > Player:** `Motion Steering` (direção por inclinação com o acelerômetro do 3DS).

---

## Local Wireless, Cheats e Mods

- **Local Wireless (LAN):** Selecione **LAN** no menu principal para correr com **2 a 6 consoles New 3DS** em qualquer modalidade. Cada console precisa do jogo e da sua própria pasta de dados convertida com a mesma versão das ferramentas. Um console cria a sala e os demais entram.
- **Códigos de Trapaça (Cheats):** Todos os códigos da versão de PC funcionam na tela inicial usando sequências de botões do 3DS. Veja a lista completa em [CHEAT_CODES.MD](CHEAT_CODES.MD).
- **Mods:** A maioria dos mods de dados de PC (modelos/texturas de carros, vinis, arte de HUD/menus, textos, sons e músicas) funciona empacotando com o **NFSU2-3DS Mod Packer** em `<GameDir>/mods/`. Mods de código (`.asi`, patches no `SPEED2.EXE`) não rodam no 3DS. Veja [MODDING.md](MODDING.md).

---

## Arquivo de Configuração (`nfsu2_config.txt`)

Fica em `sdmc:/nfsu2_config.txt` (ou ao lado do `.3dsx`). Usa o formato `Chave=Valor` (`#` inicia um comentário). É criado automaticamente na primeira execução e nunca é sobrescrito pelo jogo.

| Linha | O que faz |
| --- | --- |
| `GameDir=/nfsu2` | Caminho da pasta de dados do jogo a partir da raiz do cartão SD (**obrigatório**). |
| `Language=EN` | Idioma inicial: `EN`, `FR`, `DE`, `IT`, `ES`, `NL`, `SV`, `DA` (ou nome completo). Exige o arquivo `LANGUAGES/<Nome>.bin` correspondente em `GameDir`. Também pode ser trocado por perfil em *Options > Accessibility*. |
| `Region=US` | Região do jogo (`US`, `EU`, `KR`, `JP`, `TW`, `CN`, `TH` ou `ALL`). Veja [Regiões](#regiões). |
| `Mods=CarPack,HudSkin` | Quais mods de `mods/` carregar e em que ordem (`Mods=off` desativa todos; sem a linha, carrega todos). |
| `HwVideo=0` | Reproduz os vídeos pela CPU (VP6 por software) em vez do decodificador MVD do New 3DS. |
| `DebugLogging=1` | Grava log de depuração em `sdmc:/nfsu2_debug.txt` (desligado por padrão). |
| `DevCommands=1` | *(Desenvolvedor)* Lê comandos/capturas via `sdmc:/nfsu2_cmd.txt` (`tools/devcmd.py`) e ativa as ferramentas de debug em `L + R + SELECT`. |
| `DbgUiAudit=1` | *(Desenvolvedor)* Registra no log áreas de toque menores que 32 px e textos menores que 14 px. |

---

## Regiões

O jogo de PC padrão é a versão americana (`US`), mas todas as diferenças regionais existem no código:

| `Region=` | Versão |
| --- | --- |
| `US` (`NA`) | América do Norte (padrão) |
| `EU` (`EUROPE`, `UK`) | Europa |
| `KR` (`KOREA`) | Coreia |
| `JP` (`JAPAN`) | Japão |
| `TW` (`TAIWAN`) | Taiwan |
| `CN` (`CHINA`) | China |
| `TH` (`THAILAND`) | Tailândia |
| `ALL` | Todos os 31 carros juntos (emblemas US + 106/Corsa europeus + vinil Best Buy) |

- **Carros:** A versão `US` tem Honda Civic e Acura RSX; as demais regiões substituem esses dois pelo Peugeot 106 e pelo Corsa (quando convertidos de uma cópia de PC que os possua). `ALL` habilita todos os 31 carros.
- **Emblemas e Nomes:** Nomes e logotipos fiéis à região escolhida (ex.: Miata / MX-5 / Miata Roadster, Tiburon / Coupe / Tuscani, Vauxhall / Opel Corsa, Sprinter Trueno, Altezza, Fairlady Z, Skyline).
- **Vinil Best Buy:** Disponível em `US` e `ALL`.

---

## Compilando a partir do Código-Fonte (Build)

### Pré-requisitos

- **Toolchain 3DS do devkitPro:**
  - `3ds-dev` (`devkitARM`, `libctru`, `citro2d`, `citro3d`, `3dstools`, `picasso`)
  - `3ds-zlib` (`libz`)
- **Python 3** *(recomendado)* — executa `tools/pica_output_check.py` (evita shaders PICA200 que travam o console real) e opcionalmente recria o banner do Menu HOME (`tools/make_banner.py`). Sem Python ou sem os arquivos de PC, o banner já incluído em `3DS Files/meta/nfsu2/` é usado.
- **Nota sobre versões das bibliotecas:** O upstream original foi testado com `libctru 2.4.1`, `citro2d 1.6.0` e `citro3d 1.7.1` (e o `build_cia.ps1` exibe um aviso no Windows se outra versão estiver instalada). Este fork corrige a medição de texto em tempo de execução para `citro2d 1.7.x` e adiciona suporte ao `libctru 2.7.1` — as releases deste fork são compiladas e testadas no Linux com o devkitPro atual (`libctru 2.7.1` + `citro2d 1.7.x`).

### Compilando no Linux

1. **Instale os pacotes 3DS do devkitPro** (usando o [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman)):
   ```bash
   sudo dkp-pacman -S 3ds-dev 3ds-zlib
   ```
2. **Baixe os binários Linux de `makerom` e `bannertool`:**
   O `makerom` e o `bannertool` não vêm nos repositórios do devkitPro (o repositório inclui os `.exe` de Windows em `tools/3ds-bin/`). Baixe os binários Linux x86_64:
   - `makerom` em [releases do 3DSGuy/Project_CTR](https://github.com/3DSGuy/Project_CTR/releases)
   - `bannertool` em [releases do Steveice10/bannertool](https://github.com/Steveice10/bannertool/releases)

   Coloque ambos em `tools/3ds-bin/` e dê permissão de execução:
   ```bash
   chmod +x tools/3ds-bin/makerom tools/3ds-bin/bannertool
   ```
3. **Compile o `.3dsx` e o `.cia`:**
   Como o GNU Make com `3ds_rules` do devkitPro não funciona em caminhos com espaços (`3DS Files`), o script `tools/build_cia.sh` faz automaticamente um bind-mount de `3DS Files/` em `build_3ds/` dentro de um namespace sem privilégios (`unshare -rm`) e executa `make -j$(nproc) cia`:
   ```bash
   # Instalação padrão do devkitPro (/opt/devkitpro):
   DEVKITPRO=/opt/devkitpro bash tools/build_cia.sh

   # Ou se o devkitPro estiver em ~/.local/share/devkitpro/opt/devkitpro (padrão do script):
   bash tools/build_cia.sh
   ```
4. **Arquivos gerados:**
   - `3DS Files/NFSU2-3DS.cia`
   - `3DS Files/NFSU2-3DS.3dsx`
   - `3DS Files/NFSU2-3DS.elf`

*(Dica: você pode enviar e instalar o `.cia` compilado diretamente no New 3DS via Wi-Fi usando o **3dsDL** — veja os comandos de `curl` em [docs/bottom-screen-ui.md](docs/bottom-screen-ui.md#1-build-and-deploy-on-linux).)*

### Compilando no Windows

1. Instale o **devkitPro** pelo [devkitPro Installer](https://github.com/devkitPro/installer/releases) (marque **3DS Development**) e garanta que `3ds-zlib` esteja instalado no MSYS2 do devkitPro:
   ```bash
   pacman -S 3ds-dev 3ds-zlib
   ```
2. Os binários de Windows (`makerom.exe` e `bannertool.exe`) já estão incluídos em `tools/3ds-bin/`.
3. Na raiz do repositório, abra o **PowerShell** e execute:
   ```powershell
   powershell -ExecutionPolicy Bypass -File tools/build_cia.ps1
   ```
   O script mapeia temporariamente uma letra de unidade sem espaços (`subst`), valida os shaders, gera `3DS Files/NFSU2-3DS.3dsx` e `3DS Files/NFSU2-3DS.cia` pelo bash MSYS2 do devkitPro e remove o mapeamento ao terminar.

### Compilando as Ferramentas de PC (Opcional)

Para reconstruir os executáveis de Windows em `PC Apps/NFSU2-3DS Tools/` a partir dos scripts em `tools/`:

```bash
python tools/build_apps.py
```

### Estrutura do Repositório

- `3DS Files/` — projeto devkitARM: `source/` (código C/C++ do port e shaders PICA200), `data/` (fontes e ícones de botões embutidos), `meta/` (banner, ícone e `app.rsf` do CIA).
- `tools/` — scripts de build (`build_cia.sh`, `build_cia.ps1`), ferramentas Python de conversão/empacotamento (`stage.py`, `devcmd.py`, etc.), scripts Ghidra e `tools/3ds-bin/` (`makerom` / `bannertool`).
- `PC Apps/NFSU2-3DS Tools/` — executáveis de Windows do Data Setup e Mod Packer, código-fonte e utilitários incluídos (`ffmpeg`, `vgmstream`, `dspenc`, `tex3ds`) com suas licenças.
- `docs/` — documentação técnica do retrabalho de interface (`docs/bottom-screen-ui.md`), formatos de arquivos e comportamento original do jogo.

---

## Reportando Bugs

O jogo não gera log por padrão. Para reportar um problema:

1. Adicione `DebugLogging=1` ao arquivo `sdmc:/nfsu2_config.txt`.
2. Jogue até o problema acontecer novamente.
3. Envie o arquivo `sdmc:/nfsu2_debug.txt` junto com o crash dump do Luma3DS (caso o console tenha travado), informando a versão/release utilizada.

---

## Sobre, Créditos e Licença

- **Port Original:** Criado por [**Octoblimp**](https://github.com/Octoblimp/NFSU2-3DS).
- **Fork de UI/UX:** Mantido por [**MBaliver**](https://github.com/MBaliver/NFSU2-3DSUI).
- **Aviso Legal:** Projeto de fã sem fins lucrativos, não feito, endossado ou apoiado pela Electronic Arts. *Need for Speed* e *Need for Speed: Underground 2* são marcas registradas da Electronic Arts Inc. Este repositório não contém dados do jogo e funciona somente com arquivos de uma cópia que você possui (a única exceção é o banner, ícone e som do Menu HOME do CIA em `3DS Files/meta/nfsu2/`, gerados por `tools/make_banner.py` para permitir compilar o CIA de forma independente).
- **Licença:** O código do port é distribuído sob a **GNU General Public License v3.0** ([LICENSE](LICENSE)). O jogo, seus dados e sua arte continuam sendo propriedade da Electronic Arts. Os binários de terceiros incluídos (`makerom`, `bannertool`, `ffmpeg`, `vgmstream`, `dspenc`, `tex3ds`) mantêm suas próprias licenças.

### Transparência sobre Uso de IA

- **Port original (Octoblimp):** Claude Opus 5.5 e Claude Fable 5.2 foram utilizados para auxiliar na descompilação do jogo de PC, no port para 3DS e na depuração no Azahar, com testes em hardware real em um New Nintendo 3DS XL.
- **Fork de UI/UX (MBaliver):** Claude Opus 5.5 auxiliou na implementação e refinamento da nova interface da tela inferior, localização, acessibilidade e controles touch/movimento, em conjunto com design de UX humano, validação no Azahar e testes em hardware real no New 3DS.
