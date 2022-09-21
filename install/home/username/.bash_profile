# Instructions:
#
#   1) Copy this file to your own /home/{USERNAME}. If the file already exists,
#      just append the following two export directives.
#   2) Modify your copy. Replace /home/royconejo with your own /home/{USERNAME}
#      on both of them.
#

# embedul.ar framework base directory
# -----------------------------------
#   Path where the ./embedul.ar directory resides (not the embedul.ar directory
#   itself!) In this example path, the embedul.ar directory is located in 
#   "/home/royconejo/embedul.ar"
export LIB_EMBEDULAR_PATH="/home/royconejo"

# pip3 scripts absolute path
# --------------------------
#   This is where pip3 stores the executables (scripts) of its installed
#   applications. Needed to invoke sphinx, scan-build, etc from the command
#   line. Replace /home/royconejo with your own /home/{USERNAME}.
export PATH="$PATH:/home/royconejo/.local/bin"
