# Generates placeholder textures into GLProject/textures.
# Replace card0..card4, poster.png with your own images (same file names) when ready.
import random, math, os
from PIL import Image, ImageDraw, ImageFont, ImageFilter

OUT = os.path.join(os.path.dirname(__file__), "..", "GLProject", "textures")
os.makedirs(OUT, exist_ok=True)
random.seed(7)
FONT = "/usr/share/fonts/truetype/google-fonts/ofl/opensans/OpenSans[wdth,wght].ttf"
def font(s):
    try: return ImageFont.truetype(FONT, s)
    except Exception: return ImageFont.load_default()

def save(img, name): img.save(os.path.join(OUT, name))

# ---------- wood floor ----------
def wood(w, h, base, planks=8, vertical=False):
    img = Image.new("RGB", (w, h), base); d = ImageDraw.Draw(img)
    pw = (w if not vertical else h) // planks
    for i in range(planks):
        k = random.randint(-18, 18)
        col = tuple(max(0, min(255, c + k)) for c in base)
        for y in range(h):
            for x in range(i*pw, (i+1)*pw):
                pass
        box = (i*pw, 0, (i+1)*pw, h) if not vertical else (0, i*pw, w, (i+1)*pw)
        d.rectangle(box, fill=col)
        for _ in range(60):
            g = tuple(max(0, c - random.randint(8, 25)) for c in col)
            if not vertical:
                x = random.randint(i*pw+3, (i+1)*pw-4); y0 = random.randint(0, h); d.line([(x, y0), (x+random.randint(-2,2), y0+random.randint(40, 160))], fill=g, width=1)
            else:
                y = random.randint(i*pw+3, (i+1)*pw-4); x0 = random.randint(0, w); d.line([(x0, y), (x0+random.randint(40,160), y+random.randint(-2,2))], fill=g, width=1)
        d.rectangle(box, outline=(40, 25, 15))
    return img.filter(ImageFilter.GaussianBlur(0.6))
save(wood(512, 512, (150, 100, 60), 8), "floor.png")
save(wood(512, 512, (95, 60, 35), 5, vertical=True), "table.png")
save(wood(256, 256, (70, 45, 28), 4), "darkwood.png")

# ---------- wallpaper (damask-like pattern) ----------
img = Image.new("RGB", (256, 256), (122, 36, 52)); d = ImageDraw.Draw(img)
for cx in (0, 128, 256):
    for cy in (0, 128, 256):
        d.ellipse((cx-22, cy-30, cx+22, cy+30), outline=(214, 170, 90), width=3)
        d.ellipse((cx-8, cy-8, cx+8, cy+8), fill=(214, 170, 90))
for cx in (64, 192):
    for cy in (64, 192):
        d.polygon([(cx, cy-26), (cx+16, cy), (cx, cy+26), (cx-16, cy)], outline=(214, 170, 90), fill=(98, 26, 42))
for y in range(0, 256, 32): d.line([(0, y), (256, y)], fill=(105, 30, 46))
save(img, "wallpaper.png")

# ---------- table zones (felt with border) ----------
img = Image.new("RGB", (256, 128), (40, 90, 60)); d = ImageDraw.Draw(img)
d.rectangle((3, 3, 252, 124), outline=(230, 200, 120), width=4)
d.rectangle((12, 12, 243, 115), outline=(150, 130, 70), width=1)
save(img, "zone.png")

# ---------- cards ----------
def card(name, title, bg1, bg2, draw_fn, is_photo=False):
    W, H = 512, 768
    img = Image.new("RGB", (W, H), bg1); d = ImageDraw.Draw(img)
    for y in range(H):
        t = y / H
        d.line([(0, y), (W, y)], fill=tuple(int(bg1[i]*(1-t) + bg2[i]*t) for i in range(3)))
    d.rounded_rectangle((8, 8, W-9, H-9), 28, outline=(240, 215, 140), width=8)
    d.rounded_rectangle((26, 26, W-27, 90), 14, fill=(30, 20, 15), outline=(240, 215, 140), width=3)
    d.text((W//2, 58), title, font=font(34), fill=(245, 230, 190), anchor="mm")
    d.rounded_rectangle((26, 110, W-27, 560), 14, fill=(20, 20, 30), outline=(240, 215, 140), width=3)
    draw_fn(d, (26, 110, W-27, 560))
    d.rounded_rectangle((26, 580, W-27, H-27), 14, fill=(236, 224, 192), outline=(240, 215, 140), width=3)
    return img, d

def wizard(d, b):
    cx, cy = (b[0]+b[2])//2, (b[1]+b[3])//2
    d.ellipse((cx-90, cy-70, cx+90, cy+110), fill=(240, 205, 170))
    d.polygon([(cx-120, cy-30), (cx, cy-230), (cx+120, cy-30)], fill=(70, 50, 150))
    d.ellipse((cx-135, cy-50, cx+135, cy-10), fill=(70, 50, 150))
    d.ellipse((cx-45, cy-10, cx-15, cy+20), fill=(255, 255, 255)); d.ellipse((cx+15, cy-10, cx+45, cy+20), fill=(255, 255, 255))
    d.ellipse((cx-35, cy, cx-23, cy+14), fill=(20, 20, 20)); d.ellipse((cx+23, cy, cx+35, cy+14), fill=(20, 20, 20))
    d.polygon([(cx-80, cy+40), (cx+80, cy+40), (cx+50, cy+210), (cx, cy+260), (cx-50, cy+210)], fill=(235, 235, 235))
    d.line([(cx+150, cy-100), (cx+150, cy+200)], fill=(120, 80, 40), width=10); d.ellipse((cx+135, cy-125, cx+165, cy-95), fill=(255, 220, 80))

def dragon(d, b):
    cx, cy = (b[0]+b[2])//2, (b[1]+b[3])//2
    d.polygon([(cx-200, cy-60), (cx-60, cy-40), (cx-40, cy+60), (cx-200, cy+20)], fill=(150, 30, 30))
    d.polygon([(cx+200, cy-60), (cx+60, cy-40), (cx+40, cy+60), (cx+200, cy+20)], fill=(150, 30, 30))
    d.ellipse((cx-70, cy-30, cx+70, cy+190), fill=(200, 50, 40))
    d.ellipse((cx-60, cy-140, cx+60, cy-10), fill=(220, 60, 45))
    d.polygon([(cx-50, cy-130), (cx-30, cy-200), (cx-15, cy-130)], fill=(240, 220, 160)); d.polygon([(cx+50, cy-130), (cx+30, cy-200), (cx+15, cy-130)], fill=(240, 220, 160))
    d.ellipse((cx-35, cy-95, cx-12, cy-70), fill=(255, 220, 0)); d.ellipse((cx+12, cy-95, cx+35, cy-70), fill=(255, 220, 0))
    d.polygon([(cx-25, cy-10), (cx, cy+60), (cx+25, cy-10)], fill=(255, 140, 0))

def robot(d, b):
    cx, cy = (b[0]+b[2])//2, (b[1]+b[3])//2
    d.rectangle((cx-90, cy-110, cx+90, cy+10), fill=(150, 160, 175), outline=(230, 230, 240), width=4)
    d.rectangle((cx-130, cy+20, cx+130, cy+210), fill=(120, 130, 150), outline=(230, 230, 240), width=4)
    d.ellipse((cx-60, cy-80, cx-20, cy-40), fill=(0, 255, 200)); d.ellipse((cx+20, cy-80, cx+60, cy-40), fill=(0, 255, 200))
    d.rectangle((cx-50, cy-20, cx+50, cy-5), fill=(40, 40, 50))
    d.line([(cx, cy-110), (cx, cy-170)], fill=(230, 230, 240), width=6); d.ellipse((cx-12, cy-190, cx+12, cy-166), fill=(255, 60, 60))
    d.ellipse((cx-30, cy+70, cx+30, cy+130), fill=(0, 255, 200))

def elf(d, b):
    cx, cy = (b[0]+b[2])//2, (b[1]+b[3])//2
    d.polygon([(cx-150, cy-30), (cx-80, cy-60), (cx-70, cy)], fill=(240, 205, 170)); d.polygon([(cx+150, cy-30), (cx+80, cy-60), (cx+70, cy)], fill=(240, 205, 170))
    d.ellipse((cx-85, cy-130, cx+85, cy+60), fill=(240, 205, 170))
    d.pieslice((cx-100, cy-150, cx+100, cy+30), 180, 360, fill=(240, 220, 120))
    d.ellipse((cx-40, cy-40, cx-15, cy-10), fill=(60, 160, 90)); d.ellipse((cx+15, cy-40, cx+40, cy-10), fill=(60, 160, 90))
    d.arc((cx-30, cy-5, cx+30, cy+35), 20, 160, fill=(150, 60, 60), width=4)
    d.polygon([(cx-110, cy+60), (cx+110, cy+60), (cx+150, cy+230), (cx-150, cy+230)], fill=(40, 120, 70))
    d.line([(cx+190, cy-150), (cx+190, cy+220)], fill=(120, 80, 40), width=8); d.arc((cx+120, cy-150, cx+260, cy+220), 270, 90, fill=(200, 200, 200), width=4)

def member(d, b):
    cx, cy = (b[0]+b[2])//2, (b[1]+b[3])//2
    d.text((cx, cy-20), "PUT YOUR", font=font(44), fill=(255, 255, 255), anchor="mm")
    d.text((cx, cy+40), "PHOTO HERE", font=font(44), fill=(255, 255, 255), anchor="mm")
    d.ellipse((cx-60, cy-200, cx+60, cy-90), outline=(255, 255, 255), width=5)

specs = [
    ("card0.png", "ARCHMAGE ZELDRIN", (50, 40, 110), (20, 15, 50), wizard, "Fire Bolt  -  deals 3 damage"),
    ("card1.png", "EMBER DRAKE", (110, 30, 30), (45, 10, 10), dragon, "Flying  -  burns the enemy"),
    ("card2.png", "UNIT R-0B", (40, 70, 90), (15, 25, 40), robot, "Overclock  -  +2 attack"),
    ("card3.png", "SYLVA THE ELF", (30, 90, 60), (10, 40, 25), elf, "Volley  -  hits two targets"),
    ("card4.png", "TEAM MEMBER", (90, 70, 40), (40, 30, 15), member, "Replace card4.png with a team photo"),
]
for fname, title, c1, c2, fn, desc in specs:
    img, d = card(fname, title, c1, c2, fn)
    d.text((256, 665), desc, font=font(26), fill=(50, 35, 20), anchor="mm")
    save(img, fname)

# ---------- poster (RGBA, torn corner) ----------
W, H = 512, 700
img = Image.new("RGBA", (W, H), (0, 0, 0, 0)); d = ImageDraw.Draw(img)
base = Image.new("RGBA", (W, H), (20, 30, 80, 255)); bd = ImageDraw.Draw(base)
for y in range(H):
    t = y / H; bd.line([(0, y), (W, y)], fill=(int(20+200*t*0.4), int(30+120*t), int(80+60*(1-t)), 255))
for i in range(40):
    r = random.randint(30, 160); cx, cy = random.randint(0, W), random.randint(0, int(H*0.6))
    bd.arc((cx-r, cy-r//2, cx+r, cy+r//2), random.randint(0, 360), random.randint(0, 360), fill=(255, 220, 120, 255), width=4)
bd.ellipse((330, 60, 430, 160), fill=(255, 235, 140, 255)); bd.ellipse((360, 50, 450, 150), fill=(30, 40, 100, 255))
for x in range(0, W, 40):
    bd.polygon([(x, H), (x+20, H-260-random.randint(0, 120)), (x+40, H)], fill=(10, 20, 30, 255))
bd.rectangle((0, 0, W-1, H-1), outline=(240, 235, 220, 255), width=14)
bd.text((W//2, H-40), "STARRY HILLS", font=font(34), fill=(240, 235, 220, 255), anchor="mm")
# torn mask: irregular bottom-right corner + ragged right/bottom edge
mask = Image.new("L", (W, H), 255); md = ImageDraw.Draw(mask)
pts = [(W, H-300)]
x, y = W, H-300
for i in range(14):
    x -= random.randint(20, 55); y += random.randint(5, 45)
    if i % 2: y -= random.randint(0, 25)
    pts.append((x, min(y, H)))
pts += [(0, H+5) if False else (x, H), (W+5, H)]
md.polygon(pts, fill=0)
for i in range(0, H, 14):  # ragged left/top nibbles
    md.ellipse((-6, i, 6, i+8), fill=0)
img.paste(base, (0, 0), mask)
a = img.split()[3].point(lambda v: 255 if v > 128 else 0)
img.putalpha(a)
save(img, "poster.png")

# ---------- decor fronts ----------
# grandfather clock face
img = Image.new("RGB", (256, 768), (80, 50, 30)); d = ImageDraw.Draw(img)
d.rectangle((10, 10, 245, 757), outline=(40, 25, 15), width=6)
d.ellipse((30, 40, 226, 236), fill=(245, 235, 210), outline=(30, 20, 10), width=6)
for h in range(12):
    a = math.radians(h*30 - 90); d.line([(128+75*math.cos(a), 138+75*math.sin(a)), (128+90*math.cos(a), 138+90*math.sin(a))], fill=(20, 20, 20), width=4)
d.line([(128, 138), (128, 80)], fill=(20, 20, 20), width=6); d.line([(128, 138), (180, 160)], fill=(20, 20, 20), width=4)
d.rectangle((50, 300, 206, 650), fill=(40, 25, 15), outline=(200, 160, 70), width=4)
d.line([(128, 320), (128, 560)], fill=(200, 160, 70), width=4); d.ellipse((98, 540, 158, 600), fill=(200, 160, 70))
save(img, "clock.png")
# aquarium front
img = Image.new("RGB", (256, 128), (40, 130, 190)); d = ImageDraw.Draw(img)
for y in range(128): d.line([(0, y), (256, y)], fill=(30, 110+int(y*0.3), 180+int(y*0.2)))
d.rectangle((0, 108, 256, 128), fill=(210, 190, 130))
for (x, y, c) in [(60, 50, (255, 140, 0)), (150, 75, (255, 90, 120)), (200, 35, (255, 220, 60))]:
    d.ellipse((x, y, x+34, y+18), fill=c); d.polygon([(x+34, y+9), (x+48, y), (x+48, y+18)], fill=c)
for x in (20, 110, 230): d.line([(x, 108), (x+5, 70)], fill=(40, 160, 70), width=4)
d.rectangle((0, 0, 255, 127), outline=(220, 240, 255), width=4)
save(img, "aquarium.png")
# arcade
img = Image.new("RGB", (256, 512), (60, 30, 100)); d = ImageDraw.Draw(img)
d.rectangle((14, 14, 241, 497), outline=(255, 90, 200), width=5)
d.rectangle((36, 60, 220, 230), fill=(5, 10, 25), outline=(120, 220, 255), width=4)
d.rectangle((60, 90, 90, 110), fill=(255, 80, 80)); d.rectangle((120, 150, 150, 170), fill=(80, 255, 120)); d.ellipse((160, 90, 190, 120), fill=(255, 220, 60))
d.text((128, 30), "ARCADE", font=font(26), fill=(255, 220, 60), anchor="mm")
d.rectangle((36, 290, 220, 340), fill=(30, 20, 60)); d.ellipse((60, 295, 90, 325), fill=(255, 50, 50)); d.ellipse((110, 295, 140, 325), fill=(50, 255, 50)); d.ellipse((160, 295, 190, 325), fill=(50, 50, 255))
save(img, "arcade.png")
print("textures written to", os.path.abspath(OUT))
