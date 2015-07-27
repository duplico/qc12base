import os, sys, argparse, re, itertools

from glob import glob
from ConfigParser import ConfigParser

from PIL import Image, ImageDraw, ImageFont
import PIL.ImageOps
import PIL.ImageChops

uniform_sprite_size = 0

SPRITE_WIDTH = 64
SPRITE_HEIGHT = 64
 
HEAD_HEIGHT = 22
BODY_HEIGHT = 24
FEET_HEIGHT = 18

is_a_number = re.compile("[0-9]+")

def make_sprite(head_path, body_path, legs_path, heights=tuple(), show=False, thumb_id=False):
    global uniform_sprite_size

    sprite = Image.new('RGBA', (SPRITE_WIDTH, SPRITE_HEIGHT), (0,0,0,0))
    
    head, head_mask = adjust_image(Image.open(head_path))
    body, body_mask = adjust_image(Image.open(body_path))
    legs, legs_mask = adjust_image(Image.open(legs_path))
    
    # This is the amount of blank space at the bottom of the legs:
    squat_amount = legs.size[1] - legs_mask.getbbox()[3]
    
    # Now we place the legs; the bottom of the bounding box needs to be aligned
    # with the bottom of our sprite canvas:
    legs_y = SPRITE_HEIGHT - legs_mask.getbbox()[3]
    
    # Move the torso DOWN by the height of its corresponding legs.
    #  Then the torso will be against the bottom of the screen.
    # Then move it UP by the height of the legs we're using.
    # And DOWN by squat_amount.
    body_y = heights[3] + squat_amount - heights[2]
    
    # The head is designed to fit on something with a body+legs height
    #  of heights[4] + heights[5].
    # It's actually on something with body+legs height of heights[1]+heights[2].
    head_y = squat_amount + (heights[4]+heights[5]-heights[2]-heights[1])
    
    sd = ImageDraw.Draw(sprite)
    
    # For making lines at the demarcation points:
    #sd.line((0, 64-heights[2], 64, 64-heights[2]))
    #sd.line((0, 64-heights[2]-heights[1], 64, 64-heights[2]-heights[1]))
    
    sprite.paste(
        head, 
        ((SPRITE_WIDTH-head.size[0])/2, 
         head_y), 
        head_mask
    )
    sprite.paste(
        legs, 
        ((SPRITE_WIDTH-legs.size[0])/2, 
         legs_y), 
        legs_mask
    )
    sprite.paste(
        body, 
        ((SPRITE_WIDTH-body.size[0])/2, 
         body_y), 
        body_mask
    )
    
    if show:
        sprite.show()
    if thumb_id:
        text_image = Image.new('RGBA', (SPRITE_HEIGHT+20, SPRITE_WIDTH), (255,255,255,0))
        f = ImageFont.truetype(font="Consolas.ttf", size=10)
        td = ImageDraw.Draw(text_image)
        #td.text((0,0), "queercon", font=f, fill=(0,0,0))
        #td.text((0,54), " twelve", font=f, fill=(0,0,0))
        
        sprite_image, sprite_mask = adjust_image(sprite)
        sprite_image.save(os.path.join("thumbs", "%03d.png") % thumb_id)
        thumbnail = Image.new('RGBA', (SPRITE_WIDTH, SPRITE_HEIGHT+20), (0,0,0,0))
        f = ImageFont.truetype(font="Consolas.ttf", size=20)
        td = ImageDraw.Draw(thumbnail)
        width = td.textsize(str(thumb_id), font=f)[0]
        thumbnail.paste(text_image.rotate(-90), (0,0))
        thumbnail.paste(adjust_image(sprite)[0], (0,0), sprite_mask)
        td.text(((64-width)/2,68), str(thumb_id), font=f, fill=(0,0,0))
        thumbnail.save(os.path.join("thumbs", "label_%03d.png") % thumb_id)
            
    out_str = "{0b"
    
    index = 0
    sprite_size = 0
    
    for pixel in list(sprite.getdata()):
        if index == SPRITE_WIDTH:
            # zero-pad...
            while index % 8:
                out_str += "0"
            out_str += ", \n     0b"
            index = 0
        if index and index % 8 == 0:
            out_str += ", 0b"
        out_str += "1" if sum(pixel[:-1]) else "0" # Everything but alpha...
        index += 1
        sprite_size += 1
    out_str += "},"
    
    sprite_size = sprite_size / 8
    if uniform_sprite_size:
        assert sprite_size == uniform_sprite_size
    else:
        uniform_sprite_size = sprite_size
    
    return out_str

p_sprite_bank = []
p_sprite_pixel_bank = []
p_sprite_bank_indices = dict()
    
f_sprite_bank = []
f_sprite_pixel_bank = []
f_sprite_bank_indices = dict()

animations = []

def main(inifile, head_dir, body_dir, legs_dir, show, thumb_id=False):
    parser = ConfigParser()
    parser.read(inifile)
    head_files = sorted(glob(os.path.join(head_dir, 'head', '*.png')), key=lambda a: int(is_a_number.findall(a)[0]))
    body_files = sorted(glob(os.path.join(body_dir, 'torso', '*.png')), key=lambda a: int(is_a_number.findall(a)[0]))
    legs_files = sorted(glob(os.path.join(legs_dir, 'legs', '*.png')), key=lambda a: int(is_a_number.findall(a)[0]))
    
    heights = [HEAD_HEIGHT, BODY_HEIGHT, FEET_HEIGHT, FEET_HEIGHT, BODY_HEIGHT, FEET_HEIGHT]
    
    if os.path.isfile(os.path.join(body_dir, 'torso', 'height')):
        with open(os.path.join(body_dir, 'torso', 'height')) as height_file:
            heights[1] = int(height_file.readline())
    
    if os.path.isfile(os.path.join(legs_dir, 'legs', 'height')):
        with open(os.path.join(legs_dir, 'legs', 'height')) as height_file:
            heights[2] = int(height_file.readline())
    
    if os.path.isfile(os.path.join(body_dir, 'legs', 'height')):
        with open(os.path.join(body_dir, 'legs', 'height')) as height_file:
            heights[3] = int(height_file.readline())
    
    if os.path.isfile(os.path.join(head_dir, 'torso', 'height')):
        with open(os.path.join(head_dir, 'torso', 'height')) as height_file:
            heights[4] = int(height_file.readline())
    
    if os.path.isfile(os.path.join(head_dir, 'legs', 'height')):
        with open(os.path.join(head_dir, 'legs', 'height')) as height_file:
            heights[5] = int(height_file.readline())
    
    index_offset = 0
    index = 0
    longest_anim_buffer = 0
    
    move = 0
    
    for anim_name in parser.sections():
        # Note: The in animation must be first, followed by loop, then out.
        if ":loop" in anim_name:
            anim['loop_start_index'] = index_offset = index
        elif ":out" in anim_name:
            anim['loop_end_index'] = index_offset = index
        else:
            anim=dict(looped=(":in" in anim_name), name=anim_name.split(':')[0], images=[], persistent=False, moves=[])
            animations.append(anim)
            index_offset = 0
        # TODO: assert there's no other :s in the name?
        for frame in parser.items(anim_name):
            if is_a_number.match(frame[0]):
                # It's a frame number.
                
                if anim['persistent']:
                    sprite_bank = p_sprite_bank
                    sprite_pixel_bank = p_sprite_pixel_bank
                    sprite_bank_indices = p_sprite_bank_indices
                else:
                    sprite_bank = f_sprite_bank
                    sprite_pixel_bank = f_sprite_pixel_bank
                    sprite_bank_indices = f_sprite_bank_indices
                
                # TODO: assert that the frame numbers are correct please.
                # TODO: assert indices are in bounds
                # TODO: assert the the filename endswith index-1
                
                head_index, body_index, legs_index = map(int, frame[1].split(','))
                if (head_index, body_index, legs_index) in sprite_bank_indices:
                    frame_bank_index = sprite_bank_indices[(head_index, body_index, legs_index)]
                else:
                    frame_bank_index = len(sprite_pixel_bank)
                    sprite_bank_indices[(head_index, body_index, legs_index)] = frame_bank_index
                    pixels = make_sprite(head_files[head_index-1], body_files[body_index-1], legs_files[legs_index-1], show=show, heights=heights, thumb_id=thumb_id if anim['name']=="standing" else False)
                    metadata = "{ IMAGE_FMT_1BPP_UNCOMP, %d, %d, 2, palette_bw, %s_sprite_bank_pixels[%d] }," % (SPRITE_WIDTH, SPRITE_HEIGHT, "persistent" if anim['persistent'] else "flash", frame_bank_index)
                    metadata += " // %d:%d:%d" % (head_index, body_index, legs_index)
                    sprite_pixel_bank.append(pixels)
                    sprite_bank.append(metadata)
                    
                index = int(frame[0]) + index_offset
                img_name = "%s_%d" % (anim['name'], (index-1))
                anim['images'].append(frame_bank_index)
                anim['moves'].append(move)
                move = 0
            elif frame[0] == "persistent":
                assert not anim['images'] # Persistent MUST BE FIRST.
                anim['persistent'] = True
            elif frame[0] == "up": # 00xxxxxx - up
                assert int(frame[1]) < 50
                move = frame[1]
            elif frame[0] == "down": # 01xxxxxx - down
                assert int(frame[1]) < 50
                move = 0x40 + int(frame[1])
            elif frame[0] == "left": # 10xxxxxx - left
                move = 0x80 + int(frame[1])
                assert int(frame[1]) < 50
            elif frame[0] == "right": # 11xxxxxx - right
                move = 0xC0 + int(frame[1])
                assert int(frame[1]) < 50
        if not anim['persistent'] and index > longest_anim_buffer:
            longest_anim_buffer = index
    
    print "const uint8_t persistent_sprite_bank_pixels[%d][%d] = {\n   " % (len(p_sprite_pixel_bank), uniform_sprite_size),
    print "\n\n    ".join(p_sprite_pixel_bank)
    print "};"
    print
    
    print "const uint8_t flash_sprite_bank_pixels[%d][%d] = {\n   " % (len(f_sprite_pixel_bank), uniform_sprite_size),
    print "\n\n    ".join(f_sprite_pixel_bank)
    print "};"
    print
    
    print "const tImage persistent_sprite_bank[%d] = {\n   " % (len(p_sprite_bank)),
    print "\n    ".join(p_sprite_bank)
    print "};"
    print
    
    print "const tImage flash_sprite_bank[%d] = {\n   " % (len(f_sprite_bank)),
    print "\n    ".join(f_sprite_bank)
    print "};"
    print
    
    """
    typedef struct {
        uint8_t persistent;
        uint8_t looped;
        uint8_t loop_start;
        uint8_t loop_end;
        tImage* images[8];
    } qc12_anim_t;
    """
    
    for anim in animations:
        anim['moves'] = map(str, anim['moves'])
        bank_buffer = "persistent_sprite_bank" if anim['persistent'] else "flash_sprite_bank"
        anim['image_pointers'] = map(lambda a: "&%s[%d]" % (bank_buffer, a), anim['images'])
        print "const qc12_anim_t %s = {" % anim['name']
        print "\t%d, // Looped?" % (int(anim['looped']) if 'looped' in anim else 0)
        print "\t%d, // Loop start index" % (anim['loop_start_index'] if 'loop_start_index' in anim else 0)
        print "\t%d, // Loop end index" % (anim['loop_end_index'] if 'loop_end_index' in anim else len(anim['image_pointers']))
        print "\t%d, // Length" % len(anim['images'])
        print "\t{%s}, // Pointers to frames" % ",\n\t ".join(anim['image_pointers'])
        print "\t{%s} // Movements" % ",\n\t ".join(anim['moves'])
        print "};"
        print
        
    print "// anim_buffer_alloc = %d" % longest_anim_buffer
    print
    print "// For the animation demo:"
    print
    print "const qc12_anim_t *demo_anims[] = {"
    print "   ",
    print ", ".join(("&"+anim['name']) for anim in animations)
    print "};"
    print "const uint8_t demo_anim_count = " + str(len(animations)) + ";"

def adjust_image(image):
    assert image.mode == 'RGBA'
    a = image.split()[-1] # Alpha channel
    r = image.split()[0] # Red channel
    r_i = PIL.ImageOps.invert(r)
    return Image.merge('RGBA', (r_i,r_i,r_i,a)), a

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser(description='Generate a QC12 badge character sprite set.')
    parser.add_argument('-s', '--show', action='store_true', help="Display the"
                        " images as they are generated")
    parser.add_argument('config', help='Path to config file specifying the'
                                       'animations to make')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-i', '--id', help="Generate from badge ID", type=int)
    parts = group.add_argument_group("Body parts")
    parts.add_argument('-H', '--head', metavar='head_dir', type=str, 
                        help="Character directory to look in for the sprite's "
                             "head (must have a `head' subdirectory)")
    parts.add_argument('-B', '--body', metavar='body_dir', type=str, 
                        help="Character directory to look in for the sprite's "
                             "body (must have a `body' subdirectory)")
    parts.add_argument('-L', '--legs', metavar='legs_dir', type=str, 
                        help="Character directory to look in for the sprite's "
                             "legs (must have a `legs' subdirectory)")
    args = parser.parse_args()
    
    if args.id:
        assert args.id >= 15 # ubers are done manually.
        c = ["bender", "bear", "human", "lizard", "octopus", "robot"]
        l = list(itertools.product(c, repeat=3))
        head, body, legs = l[(args.id-15) % len(l)]
    
    main(args.config, args.head or head, args.body or body, args.legs or legs, args.show, thumb_id=args.id)