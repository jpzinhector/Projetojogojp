import os
import sys
from PIL import Image

RAIZ = os.path.dirname(os.path.abspath(__file__))
PROJETO = os.path.dirname(RAIZ)
SAIDA = os.path.join(PROJETO, "Codigo", "assets", "orcs")
ANIMACOES = ["IDLE", "WALK", "ATTAK", "HURT", "DIE"]
QUADROS = 10
LARGURA_MAX = 400


def caminho(numero, animacao, indice):
    return os.path.join(PROJETO, "%d_ORK" % numero, "ORK_%02d_%s_%03d.png" % (numero, animacao, indice))


def carregar(numero):
    dados = {}
    for animacao in ANIMACOES:
        arquivos = [caminho(numero, animacao, i) for i in range(QUADROS)]
        if all(os.path.exists(a) for a in arquivos):
            dados[animacao] = [Image.open(a).convert("RGBA") for a in arquivos]
    return dados


def unir_caixas(dados):
    caixa = None
    for quadros in dados.values():
        for q in quadros:
            b = q.getchannel("A").getbbox()
            if b is None:
                continue
            caixa = b if caixa is None else (min(caixa[0], b[0]), min(caixa[1], b[1]), max(caixa[2], b[2]), max(caixa[3], b[3]))
    return caixa


def referencia(dados):
    for nome in ("IDLE", "ATTAK", "WALK"):
        if nome in dados:
            return dados[nome][0]
    return None


def centro_de_massa_x(quadro):
    alfa = quadro.getchannel("A").point(lambda v: 255 if v > 128 else 0)
    caixa = alfa.getbbox()
    corte = alfa.crop((caixa[0], caixa[1] + (caixa[3] - caixa[1]) // 3, caixa[2], caixa[3]))
    pixels = corte.load()
    soma = 0
    total = 0
    for y in range(corte.height):
        for x in range(corte.width):
            if pixels[x, y]:
                soma += x
                total += 1
    return caixa[0] + soma / max(total, 1)


def gerar(numero):
    dados = carregar(numero)
    if not dados:
        print("orc %d: sem quadros, ignorado" % numero)
        return
    caixa = unir_caixas(dados)
    ref = referencia(dados)
    caixa_ref = ref.getchannel("A").getbbox()
    largura = caixa[2] - caixa[0]
    altura = caixa[3] - caixa[1]
    escala = min(0.40, LARGURA_MAX / float(largura))
    lq = int(round(largura * escala))
    aq = int(round(altura * escala))

    ancora_x = (centro_de_massa_x(ref) - caixa[0]) * escala
    ancora_y = (caixa_ref[3] - caixa[1]) * escala
    altura_ref = (caixa_ref[3] - caixa_ref[1]) * escala

    os.makedirs(SAIDA, exist_ok=True)
    for animacao, quadros in dados.items():
        tira = Image.new("RGBA", (lq * QUADROS, aq), (0, 0, 0, 0))
        for i, q in enumerate(quadros):
            tira.paste(q.crop(caixa).resize((lq, aq), Image.LANCZOS), (i * lq, 0))
        tira.save(os.path.join(SAIDA, "orc%d_%s.png" % (numero, animacao.lower())), optimize=True)

    with open(os.path.join(SAIDA, "orc%d.txt" % numero), "w") as f:
        f.write("%d %d %.2f %.2f %.2f\n" % (lq, aq, ancora_x, ancora_y, altura_ref))
    print("orc %d: %s quadro=%dx%d ancora=(%.1f,%.1f) alturaRef=%.1f" % (numero, ",".join(dados), lq, aq, ancora_x, ancora_y, altura_ref))


if __name__ == "__main__":
    numeros = [int(a) for a in sys.argv[1:]] or [1, 2, 3]
    for n in numeros:
        gerar(n)
