import os, sys, argparse, re
from glob import glob
from ConfigParser import ConfigParser

from PIL import Image
import PIL.ImageOps

SPRITE_WIDTH = 64
SPRITE_HEIGHT = 64
 
HEAD_HEIGHT = 22
BODY_HEIGHT = 46
FEET_HEIGHT = 18

is_a_number = re.compile("[0-9]+")

def make_sprite(head_path, body_path, legs_path):
    sprite = Image.new('RGBA', (SPRITE_WIDTH, SPRITE_HEIGHT), (0,0,0,0))
    
    head, head_mask = adjust_image(Image.open(head_path))
    body, body_mask = adjust_image(Image.open(body_path))
    legs, legs_mask = adjust_image(Image.open(legs_path))
    
    feet_offset = FEET_HEIGHT - legs_mask.getbbox()[3]
    body_offset = BODY_HEIGHT - body_mask.getbbox()[3]
    
    sprite.paste(head, ((SPRITE_WIDTH-head.size[0])/2, 0+feet_offset+body_offset), head_mask)    
    sprite.paste(body, (0,0+feet_offset+body_offset), body_mask)
    sprite.paste(legs, ((SPRITE_WIDTH-legs.size[0])/2, body.size[1]+feet_offset), legs_mask)
            
    out_str = "{0b"
    
    index = 0
    
    for pixel in list(sprite.getdata()):
        if index == SPRITE_WIDTH:
            # zero-pad...
            while index % 8:
                out_str += "0"
            out_str += ", \n     0b"
            index = 0
        if index and index % 8 == 0:
            out_str += ", 0b"
        out_str += "1" if sum(pixel) else "0"
        index += 1
    out_str += "},"
    
    return out_str

p_sprite_bank = []
p_sprite_pixel_bank = []
p_sprite_bank_indices = dict()
    
f_sprite_bank = []
f_sprite_pixel_bank = []
f_sprite_bank_indices = dict()

animations = []

def main(inifile, head_dir, body_dir, legs_dir):
    parser = ConfigParser()
    parser.read(inifile)
    head_files = glob(os.path.join(head_dir, 'head', '*[0-9].png'))
    body_files = glob(os.path.join(head_dir, 'body', '*[0-9].png'))
    legs_files = glob(os.path.join(head_dir, 'legs', '*[0-9].png'))
    index_offset = 0
    index = 0
    longest_anim_buffer = 0
    for anim_name in parser.sections():
        # Note: The in animation must be first, followed by loop, then out.
        if ":loop" in anim_name:
            anim['loop_start_index'] = index_offset = index
        elif ":out" in anim_name:
            anim['loop_end_index'] = index_offset = index
        else:
            anim=dict(looped=(":in" in anim_name), name=anim_name.split(':')[0], images=[], persistent=False)
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
                    pixels = make_sprite(head_files[head_index-1], body_files[body_index-1], legs_files[legs_index-1])
                    metadata = "{ IMG_FMT_1BPP_UNCOMP, %d, %d, 2, palette_bw, %s }," % (SPRITE_WIDTH, SPRITE_HEIGHT, "bank[%d]" % frame_bank_index)
                    metadata += " // %d:%d:%d" % (head_index, body_index, legs_index)
                    sprite_pixel_bank.append(pixels)
                    sprite_bank.append(metadata)
                    
                index = int(frame[0]) + index_offset
                img_name = "%s_%d" % (anim['name'], (index-1))
                anim['images'].append(frame_bank_index)
            elif frame[0] == "persistent":
                assert not anim['images'] # Persistent MUST BE FIRST.
                anim['persistent'] = True
        if not anim['persistent'] and index > longest_anim_buffer:
            longest_anim_buffer = index
    
    print "persistent_sprite_bank_pixels = {\n   ",
    print "\n\n    ".join(p_sprite_pixel_bank)
    print "}"
    
    print "flash_sprite_bank_pixels = {\n   ",
    print "\n\n    ".join(f_sprite_pixel_bank)
    print "}"
    
    print "persistent_sprite_bank = {\n   ",
    print "\n    ".join(p_sprite_bank)
    print "}"
    
    print "flash_sprite_bank = {\n   ",
    print "\n    ".join(f_sprite_bank)
    print "}"
    
    for anim in animations:
        print anim
        print
        print
        
    print "anim_buffer_alloc = %d" % longest_anim_buffer
        

def adjust_image(image):
    assert image.mode == 'RGBA'
    a = image.split()[-1]
    return Image.merge('RGBA', (a,a,a,a)), a

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('config', help='Path to config file specifying the'
                                       'animations to make')
    parser.add_argument('head', metavar='head_dir', type=str, 
                        help="Character directory to look in for the sprite's "
                             "head (must have a `head' subdirectory)")
    parser.add_argument('body', metavar='body_dir', type=str, 
                        help="Character directory to look in for the sprite's "
                             "body (must have a `body' subdirectory)")
    parser.add_argument('legs', metavar='legs_dir', type=str, 
                        help="Character directory to look in for the sprite's "
                             "legs (must have a `legs' subdirectory)")
    args = parser.parse_args()

    main(args.config, args.head, args.body, args.legs)