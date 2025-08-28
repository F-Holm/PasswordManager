require_linux("GTK")

# Dependencias
add_subdirectory(meson) # Tiene que estar primero
add_subdirectory(cairo)
add_subdirectory(epoxy)
add_subdirectory(gdk-pixbuf)
add_subdirectory(glib)
add_subdirectory(gobject-introspection)
add_subdirectory(graphene)
add_subdirectory(pango)
add_subdirectory(xkb-common)

# Build