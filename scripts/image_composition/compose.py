import os, sys, argparse
from glob import glob
from ConfigParser import ConfigParser

from PIL import Image
import PIL.ImageOps

SPRITE_WIDTH = 64
SPRITE_HEIGHT = 64
 
HEAD_HEIGHT = 22
BODY_HEIGHT = 46
FEET_HEIGHT = 18

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
            out_str += ", \n 0b"
            index = 0
        if index and index % 8 == 0:
            out_str += ", 0b"
        out_str += "1" if sum(pixel) else "0"
        index += 1
    out_str += "}"
    
    return out_str
    

def main(inifile, head_dir, body_dir, legs_dir):
    parser = ConfigParser()
    parser.read(inifile)
    head_files = glob(os.path.join(head_dir, 'head', '*[0-9].png'))
    body_files = glob(os.path.join(head_dir, 'body', '*[0-9].png'))
    legs_files = glob(os.path.join(head_dir, 'legs', '*[0-9].png'))
    for anim_name in parser.sections():
        for frame in parser.items(anim_name):
            # TODO: assert that the frame numbers are correct please.
            head_index, body_index, legs_index = map(int, frame[1].split(','))
            # TODO: assert indices are in bounds
            # TODO: assert the the filename endswith index-1
            pixels = make_sprite(head_files[head_index-1], body_files[body_index-1], legs_files[legs_index-1])
            
            img_name = "%s_%s" % (anim_name, frame[0])
            print "static const unsigned char %s_pixels[] = \n%s;" % (img_name, pixels)
            print ""
            print "const tImage %s = {" % img_name
            print "    IMAGE_FMT_1BPP_UNCOMP,"
            print "    %d," % SPRITE_WIDTH
            print "    %d," % SPRITE_HEIGHT
            print "    2,"
            print "    palette_bw,"
            print "    %s_pixels," % img_name
            print "};"
            print
            print
        

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