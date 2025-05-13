from os.path import join
Import("env")  # Import only 'env', not 'projenv'

# Custom BIN from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([  # This runs the post-build action to convert ELF to BIN
                "$OBJCOPY",
                "-O",
                "binary",
                "$TARGET",
                "$BUILD_DIR/${PROGNAME}.bin"
            ]), "Building $TARGET"))
