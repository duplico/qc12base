import os, sys, argparse, re, itertools

from collections import OrderedDict

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

DEFAULT_SPEED = 3

is_a_number = re.compile("[0-9]+")

def get_bits(paths):
    segs = []
    for path in paths:
        head_in, head_mask = adjust_image(Image.open(path))
        seg = Image.new('RGBA', (SPRITE_WIDTH, SPRITE_HEIGHT), (0,0,0,0))
        seg.paste(head_in, (0, 0), head_mask)
        
        out_str = "{0b"
        index = 0
        sprite_size = 0
        for pixel in list(seg.getdata()):
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
        segs.append(out_str)
    return segs
    

def make_outputs(head_paths, body_paths, legs_paths):
    parts = dict(
        heads = get_bits(head_paths),
        bodies = get_bits(body_paths),
        legs = get_bits(legs_paths)
    )
    
    for partname, bits in parts.items():
        print 'const uint8_t %s_pixels[%d][512] = {' % (partname, len(bits))
        print '   ',
        print '\n    '.join(bits)
        print '};'
    
    print
    
    for partname, bits in parts.items():
        print "const tImage %s[%d] = {" % (partname, len(bits))
        for i in range(len(bits)):
            print "   {IMAGE_FMT_1BPP_UNCOMP, 64, 64, 2, palette_bw, %s_pixels[%d]}," % (
                partname,
                i
            )
        print '};'
        print
    
    return parts['heads'], parts['bodies'], parts['legs']

def get_tops(head_path, body_path, legs_path, heights):
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
    
    return head_y, body_y, legs_y

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
    
    make_outputs(head_files, body_files, legs_files) # Prints directly.
    
    
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
            anim = dict(looped=(":in" in anim_name), name=anim_name.split(':')[0], parts=OrderedDict(heads=[], bodies=[], legs=[]), offsets=OrderedDict(head=[], body=[], legs=[]), moves=[])
            anim['speed'] = DEFAULT_SPEED
            animations.append(anim)
            index_offset = 0
        for frame in parser.items(anim_name):
            if is_a_number.match(frame[0]):
                # It's a frame number.
                
                # TODO: assert that the frame numbers are correct please.
                # TODO: assert indices are in bounds
                # TODO: assert the the filename endswith index-1
                
                head_index, body_index, legs_index = map(int, frame[1].split(','))
                anim['parts']['heads'].append(head_index-1)
                anim['parts']['bodies'].append(body_index-1)
                anim['parts']['legs'].append(legs_index-1)
                
                h,b,t = get_tops(head_files[head_index-1], body_files[body_index-1], legs_files[legs_index-1], heights)
                
                anim['offsets']['head'].append(h)
                anim['offsets']['body'].append(b)
                anim['offsets']['legs'].append(t)
                
                index = int(frame[0]) + index_offset
                anim['moves'].append(move)
                move = 0
            elif frame[0].startswith("up"): # 00xxxxxx - up
                assert int(frame[1]) < 65
                move = frame[1]
            elif frame[0].startswith("down"): # 01xxxxxx - down
                assert int(frame[1]) < 65
                move = 0x40 + int(frame[1])
            elif frame[0].startswith("left"): # 10xxxxxx - left
                move = 0x80 + int(frame[1])
                assert int(frame[1]) < 65
            elif frame[0].startswith("right"): # 11xxxxxx - right
                move = 0xC0 + int(frame[1])
                assert int(frame[1]) < 65
            elif frame[0] == "speed":
                anim['speed'] = int(frame[1])
        if index > longest_anim_buffer:
            longest_anim_buffer = index
    
    for anim in animations:
        assert len(anim['parts']['heads']) == len(anim['parts']['bodies']) == len(anim['parts']['legs']) == len(anim['moves'])
        anim['len'] = len(anim['parts']['heads'])
    
    """
    typedef struct {
        uint8_t looped;
        uint8_t loop_start;
        uint8_t loop_end;
        uint8_t len;
        uint8_t speed;
        const tImage** head_images;
        const tImage** body_images;
        const tImage** legs_images;
        const uint8_t* head_tops;
        const uint8_t* body_tops;
        const uint8_t* legs_tops;
        const uint8_t* movement;
    } qc12_anim_t;
    """
    
    print "uint8_t no_moves[%d] = {%s};" % (longest_anim_buffer, '0')
    print
    
    for anim in animations:
        no_moves = all(i == 0 for i in anim['moves'])
        anim['moves'] = map(str, anim['moves'])
        
        print "// " + anim['name']
        
        for partname, indices in anim['parts'].items():
            print "const uint8_t %s_%s[%d] = {" % (anim['name'], partname, anim['len']),
            print "" + ', '.join(map(str, indices)),
            print "};"
        
        for partname, tops in anim['offsets'].items():
            print "const int8_t %s_%s_tops[%d] = {" % (anim['name'], partname, anim['len']),
            print "" + ', '.join(map(str, tops)),
            print "};"
                
        if not no_moves:
            print "const uint8_t %s_moves[%d] = { %s };" % (
                anim['name'],
                anim['len'],
                ", ".join(anim['moves'])
            )
            print ""
        
        print "const qc12_anim_t %s = {" % anim['name']
        print "\t%d, // Looped?" % (int(anim['looped']) if 'looped' in anim else 0)
        print "\t%d, // Loop start index" % (anim['loop_start_index'] if 'loop_start_index' in anim else 0)
        print "\t%d, // Loop end index" % (anim['loop_end_index'] if 'loop_end_index' in anim else anim['len'])
        print "\t%d, // Length" % anim['len']
        print "\t%d, // Speed" % anim['speed']
        for partname in ['heads', 'bodies', 'legs']:
            print "\t%s_%s, //%s" % (anim['name'], partname, partname)
        for partname in ['head', 'body', 'legs']:
            print "\t%s_%s_tops, //%s" % (anim['name'], partname, partname)
        if no_moves:
            print "\tno_moves, // Moves"
        else:
            print "\t%s_moves, // Moves" % anim['name']
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
    parser.add_argument('-f', '--fixini', action='store_true', help="Fix the"
                        " format of the ini file.")
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
    
    use_file = args.config
    
    if args.fixini:
        filestring = ""
        with open(args.config) as configfile:
            for line in configfile:
                filestring += line
        left_num = 0
        right_num = 0
        up_num = 0
        down_num = 0
        # TODO: should not match animation names.
        # TODO: this should search for the highest one.
        while filestring.find("left: ") != -1 or \
              filestring.find("right: ") != -1 or \
              filestring.find("up: ") != -1 or \
              filestring.find("down: ") != -1:
            filestring = filestring.replace("left: ", "left%d: " % left_num, 1)
            filestring = filestring.replace("right: ", "right%d: " % right_num, 1)
            filestring = filestring.replace("up: ", "up%d: " % up_num, 1)
            filestring = filestring.replace("down: ", "down%d: " % down_num, 1)
            left_num = right_num = up_num = down_num = down_num+1
            
        with open(args.config + '.tmp', 'w') as configfile:
            configfile.write(filestring)
        use_file = args.config + '.tmp'
    
    if args.id:
        assert args.id >= 15 # ubers are done manually.
        c = ["alien", "bear", "human", "lizard", "octopus", "robot"]
        l = list(itertools.product(c, repeat=3))
        head, body, legs = l[(args.id-15) % len(l)]
    
    main(use_file, args.head or head, args.body or body, args.legs or legs, args.show, thumb_id=args.id)
    
    if args.fixini:
        os.remove(use_file)