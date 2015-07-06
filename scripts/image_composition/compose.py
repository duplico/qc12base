import os, sys, argparse, re
from glob import glob
from ConfigParser import ConfigParser

from PIL import Image
import PIL.ImageOps

uniform_sprite_size = 0

SPRITE_WIDTH = 64
SPRITE_HEIGHT = 64
 
HEAD_HEIGHT = 22
BODY_HEIGHT = 24
FEET_HEIGHT = 18

is_a_number = re.compile("[0-9]+")

def make_sprite(head_path, body_path, legs_path, heights=(HEAD_HEIGHT, BODY_HEIGHT, FEET_HEIGHT)):
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
    
    # The baseline leg height is FEET_HEIGHT. So we need to lower the torso by
    # FEET_HEIGHT - heights[2] + squat_amount
    body_y = squat_amount + (FEET_HEIGHT - heights[2])

    # Now we just plop the head on at SPRITE_HEIGHT - the heights of everything
    # plus the squat amount:
    head_y = squat_amount + (SPRITE_HEIGHT - sum(heights)) # TODO: might be wrong...
    
    sprite.paste(
        head, 
        ((SPRITE_WIDTH-head.size[0])/2, 
         head_y), 
        head_mask
    )
    sprite.paste(
        body, 
        ((SPRITE_WIDTH-body.size[0])/2, 
         body_y), 
        body_mask
    )
    sprite.paste(
        legs, 
        ((SPRITE_WIDTH-legs.size[0])/2, 
         legs_y), 
        legs_mask
    )
    
    sprite.show()
            
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
        out_str += "1" if sum(pixel) else "0"
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

def main(inifile, head_dir, body_dir, legs_dir):
    parser = ConfigParser()
    parser.read(inifile)
    head_files = glob(os.path.join(head_dir, 'head', '*.png'))
    body_files = glob(os.path.join(body_dir, 'body', '*.png'))
    legs_files = glob(os.path.join(legs_dir, 'legs', '*.png'))
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
                    metadata = "{ IMAGE_FMT_1BPP_UNCOMP, %d, %d, 2, palette_bw, %s_sprite_bank_pixels[%d] }," % (SPRITE_WIDTH, SPRITE_HEIGHT, "persistent" if anim['persistent'] else "flash", frame_bank_index)
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
    
    print "uint8_t persistent_sprite_bank_pixels[%d][%d] = {\n   " % (len(p_sprite_pixel_bank), uniform_sprite_size),
    print "\n\n    ".join(p_sprite_pixel_bank)
    print "};"
    print
    
    print "flash_sprite_bank_pixels[%d][%d] = {\n   " % (len(f_sprite_pixel_bank), uniform_sprite_size),
    print "\n\n    ".join(f_sprite_pixel_bank)
    print "};"
    print
    
    print "tImage persistent_sprite_bank[%d] = {\n   " % (len(p_sprite_bank)),
    print "\n    ".join(p_sprite_bank)
    print "};"
    print
    
    print "tImage flash_sprite_bank[%d] = {\n   " % (len(f_sprite_bank)),
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
        bank_buffer = "persistent_sprite_bank" if anim['persistent'] else "flash_sprite_bank"
        anim['image_pointers'] = map(lambda a: "&%s[%d]" % (bank_buffer, a), anim['images'])
        print "qc12_anim_t %s = {" % anim['name']
        print "\t%d, // Looped?" % (int(anim['looped']) if 'looped' in anim else 0)
        print "\t%d, // Loop start index" % (anim['loop_start_index'] if 'loop_start_index' in anim else 0)
        print "\t%d, // Loop end index" % (anim['loop_end_index'] if 'loop_end_index' in anim else len(anim['image_pointers']))
        print "\t%d, // Length" % len(anim['images'])
        print "\t{%s} // Pointers to frames" % ",\n\t ".join(anim['image_pointers'])
        print "};"
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