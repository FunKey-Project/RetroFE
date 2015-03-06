import argparse
import errno
import glob
import os
import shutil
import sys

def copytree(src, dst):
  if os.path.isdir(src):
      if not os.path.exists(dst):
          os.makedirs(dst)
      for name in os.listdir(src):
          copytree(os.path.join(src, name),
                   os.path.join(dst, name))
  else:
      shutil.copyfile(src, dst)
      
def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise
#todo: this script needs to be broken up into multiple methods 
#      and should be ported to be more like a class

#####################################################################
# Parse arguments
#####################################################################
parser = argparse.ArgumentParser(description='Bundle up some RetroFE common files.')
parser.add_argument('--os', choices=['windows','linux'], required=True, help='Operating System (windows or linux)')
parser.add_argument('--gstreamer_path', help='path to gstreamer sdk (i.e. D:/gstreamer')
parser.add_argument('--build', default='full', help='define what contents to package (full, core, engine, layout,none')
parser.add_argument('--clean', action='store_true', help='clean the output directory')

args = parser.parse_args()

gstreamer_path = None

# windows needs a gstreamer path set
if args.os == 'windows':
  if not hasattr(args, 'gstreamer_path'):
    print 'missing argument --gstreamer_path '
    sys.exit(-1)
    
  gstreamer_path = args.gstreamer_path
  
  if not os.path.exists(gstreamer_path):
    print 'could not find gstreamer libraries: ' + gstreamer_path
    sys.exit(-1)


#####################################################################
# Determine base path os to build
#####################################################################
base_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
common_path = os.path.join(base_path, 'Package', 'Environment', 'Common')
os_path = None

if args.os == 'windows':
  os_path = os.path.join(base_path, 'Package', 'Environment', 'Windows')
  
elif args.os == 'linux':
  os_path = os.path.join(base_path, 'Package', 'Environment', 'Linux')


#####################################################################
# Copy layers, artwork, config files, etc for the given os
#####################################################################
output_path = os.path.join(base_path, 'artifacts', args.os, 'RetroFE')

if os.path.exists(output_path) and hasattr(args, 'clean'):
  shutil.rmtree(output_path)


if args.build != 'none' and not os.path.exists(output_path):
  os.makedirs(output_path)

if args.build == 'full':
  copytree(common_path, output_path)
  copytree(os_path, output_path)
  for collection in os.walk(os.path.join(output_path, 'collections')).next()[1]:
    if not collection.startswith('_'):
     mkdir_p(os.path.join(output_path, 'collections', collection, 'roms'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'artwork_front'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'artwork_back'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'medium_back'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'medium_front'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'bezel'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'logo'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'screenshot'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'screentitle'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'system_artwork'))
 
elif args.build == 'layout':
  layout_dest_path = os.path.join(output_path, 'layouts')
  layout_common_path = os.path.join(common_path, 'layouts')
  layout_os_path = os.path.join(os_path, 'layouts')
  
  if not os.path.exists(layout_dest_path):
    os.makedirs(layout_dest_path)

  if os.path.exists(layout_common_path):
    copytree(layout_common_path, layout_dest_path)
    
  if os.path.exists(layout_os_path):
    copytree(layout_os_path, layout_dest_path)

#####################################################################
# Copy retrofe executable
#####################################################################
if args.os == 'windows':
  if args.build == 'full' or args.build == 'core' or args.build == 'engine':
    # copy retrofe.exe to core folder
    src_exe = os.path.join(base_path, 'RetroFE', 'Build', 'Release', 'retrofe.exe')
    core_path = os.path.join(output_path, 'core')
    
    # create the core folder
    if not os.path.exists(core_path):
      os.makedirs(core_path)
      
    # copy retrofe.exe
    shutil.copy(src_exe, core_path)
    third_party_path = os.path.join(base_path, 'RetroFE', 'ThirdParty')
        
  if args.build == 'full' or args.build == 'core':
    libraries = [
      os.path.join(gstreamer_path, 'lib/*.dll'),
      os.path.join(gstreamer_path, 'lib/gstreamer-1.0/*.dll'),
      os.path.join(gstreamer_path, 'bin/*.dll'),
      os.path.join(third_party_path, 'SDL2-2.0.3', 'lib', 'x86', r'*.dll'),
      os.path.join(third_party_path, 'SDL2_image-2.0.0', 'lib', 'x86', '*.dll'),
      os.path.join(third_party_path, 'SDL2_mixer-2.0.0', 'lib', 'x86', '*.dll'),
      os.path.join(third_party_path, 'SDL2_ttf-2.0.12', 'lib', 'x86', '*.dll')
    ]
    
    # copy third party libraries
    for glob_dlls in libraries:
      for file in glob.glob(glob_dlls):
        shutil.copy(file, core_path)

  
elif args.os == 'linux':
  if args.build == 'full' or args.build == 'core' or args.build == 'engine':
    src_exe = os.path.join(base_path, 'RetroFE', 'Source', 'Build', 'retrofe')
    shutil.copy(src_exe, output_path)




