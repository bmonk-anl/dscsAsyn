
#ifndef DSCS_EXPORT_H__
#define DSCS_EXPORT_H__

#ifdef DSCS_STATIC
#  define DSCS_EXPORT
#  define DSCS_NO_EXPORT
#else
#  ifndef DSCS_EXPORT
#    ifdef dscs_EXPORTS
        /* We are building this library */
#      define DSCS_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define DSCS_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef DSCS_NO_EXPORT
#    define DSCS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef DSCS_DEPRECATED
#  define DSCS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef DSCS_DEPRECATED_EXPORT
#  define DSCS_DEPRECATED_EXPORT DSCS_EXPORT DSCS_DEPRECATED
#endif

#ifndef DSCS_DEPRECATED_NO_EXPORT
#  define DSCS_DEPRECATED_NO_EXPORT DSCS_NO_EXPORT DSCS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef DSCS_NO_DEPRECATED
#    define DSCS_NO_DEPRECATED
#  endif
#endif

#endif /* DSCS_EXPORT_H__ */
