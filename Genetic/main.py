from random import randint, random
from PIL import Image, ImageDraw


def open_images(image_info):
    images_load = []
    for idx in range(len(image_info)):
        img = Image.open(image_info[idx][0]).convert('RGBA')
        images_load.append(img)
    return images_load


def count_pixels(images_load, image_info):
    for idx in range(len(image_info)):
        cnt = 0
        pixels = images_load[idx].load()
        for i in range(images_load[idx].size[0]):
            for j in range(images_load[idx].size[1]):
                if pixels[i, j][3] > 200:
                    cnt += 1
        image_info[idx][2] = cnt


def autocrop_image(image):
    bbox = image.getbbox()
    image = image.crop(bbox)
    (width, height) = image.size
    cropped_image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    cropped_image.paste(image, (0, 0))
    return cropped_image


def get_parents(parent_num, pop_size, old_fit):
    parents = [0] * parent_num
    for i in range(parent_num):
        parents[i] = randint(0, pop_size - 1)
        for j in range(parent_num - 1):
            idx = randint(0, pop_size - 1)
            if old_fit[idx] > old_fit[parents[i]]:
                parents[i] = idx
    return parents


def fitness(binary, bits, canvas_width, canvas_height, image_info, angle_h, images_load, save = False):
    unfit = 0
    start = 0
    image = Image.new('RGBA', (canvas_width, canvas_height), (255, 0, 0, 0))
    new_pixels = image.load()
    for idx in range(len(image_info)):
        img = images_load[idx]
        for cnt in range(image_info[idx][1]):
            rot_img = autocrop_image(img.rotate(int(''.join(binary[start:start + bits]), 2) * angle_h, expand=True))
            pixels = rot_img.load()
            x = int(''.join(binary[start + bits:start + 2 * bits]), 2) * (
                    canvas_width - rot_img.size[0]) / (2 ** bits - 1)
            start += 2 * bits
            y = 0
            ok = False
            while not ok and y <= canvas_height - rot_img.size[1]:
                bad = False
                for i in range(rot_img.size[0]):
                    if bad:
                        break
                    for j in range(rot_img.size[1]):
                        if pixels[i, j][3] > 200:
                            if new_pixels[x + i, y + j][3] < 50:
                                new_pixels[x + i, y + j] = pixels[i, j]
                            else:
                                for k in range(i):
                                    for l in range(rot_img.size[1]):
                                        if pixels[k, l][3] > 200:
                                            new_pixels[x + k, y + l] = (255, 0, 0, 0)
                                for k in range(j):
                                    if pixels[i, k][3] > 200:
                                        new_pixels[x + i, y + k] = (255, 0, 0, 0)
                                y += 1
                                bad = True
                                break
                if bad:
                    continue
                ok = True
            if not ok:
                unfit += image_info[idx][2]
    if save:
        image.save('best.png', 'PNG')
    return -unfit


def int_crop(string):
    return int(string[:-1:])


file = open("parameters.txt", "r")
lines = file.readlines()
file.close()
bits, pop_size, gen_num, parent_num, canvas_width, canvas_height = map(int_crop, lines[1:12:2])
prob = lines[13][:-1:]
image_info = [[lines[i].split()[0], int(lines[i].split()[1]), 0] for i in range(15, len(lines))]
images_load = open_images(image_info)
count_pixels(images_load, image_info)
image_num = 0
for inf in image_info:
    image_num += inf[1]
if prob == "low":
    prob = 1 / (3 * bits)
elif prob == "average":
    prob = 1 / bits
else:
    prob = 3 / bits
angle_h = 360 / (2 ** bits - 1)
binary_len = 2 * image_num * bits
max_fit = -2 * 10 ** 9
best = []
old_gen = [['0'] * binary_len] * pop_size
new_gen = old_gen[::]
old_fit = [0] * pop_size
new_fit = old_fit[::]
for i in range(pop_size):
    for j in range(binary_len):
        old_gen[i][j] = str(randint(0, 1))
    old_fit[i] = fitness(old_gen[i], bits, canvas_width, canvas_height, image_info, angle_h, images_load)
    if old_fit[i] > max_fit:
        max_fit = old_fit[i]
        best = old_gen[i][::]
for cnt in range(gen_num):

    print("Generation № {}".format(cnt + 1))
    print("Remaining time: {:.2f}%".format(100 - 100 * cnt / gen_num))
    print("Best fitness: {}".format(max_fit))
    print("--------")

    for i in range(pop_size):
        parents = get_parents(parent_num, pop_size, old_fit)
        start = 0
        for j in range(parent_num - 1):
            end = randint(start + 1, binary_len - parent_num + j + 1)
            new_gen[i][start:end] = old_gen[parents[j]][start:end]
            start = end
        new_gen[i][start:] = old_gen[parents[parent_num - 1]][start:]
        for j in range(binary_len):
            if random() < prob:
                new_gen[i][j] = '0' if new_gen[i][j] == '1' else '1'
        new_fit[i] = fitness(new_gen[i], bits, canvas_width, canvas_height, image_info, angle_h, images_load)
        if new_fit[i] > max_fit:
            max_fit = new_fit[i]
            best = new_gen[i][::]
    old_gen, new_gen = new_gen, old_gen
    old_fit, new_fit = new_fit, old_fit
fitness(best, bits, canvas_width, canvas_height, image_info, angle_h, images_load, True)
print(f"\nBest solution: {max_fit}")
