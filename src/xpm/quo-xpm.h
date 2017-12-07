/*
 * Copyright (c) 2017      Los Alamos National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the libquo project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

/**
 * @file quo-xpm.h
 */

#ifndef QUO_XPM_H_INCLUDED
#define QUO_XPM_H_INCLUDED

#include "quo.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque QUO XPM context. */
struct quo_xpm_t;
typedef struct quo_xpm_t quo_xpm_t;
/** External QUO XPM context type. */
typedef quo_xpm_t * QUO_xpm_context;
// TODO(skg) consider making this opaque once interface is complete. */
typedef struct QUO_xpm_view_t {
    void *base;
    size_t extent;
} QUO_xpm_view_t;

int
QUO_xpm_allocate(QUO_context qc,
                 size_t local_size,
                 QUO_xpm_context *new_xpm);

int
QUO_xpm_free(QUO_xpm_context xpm);

int
QUO_xpm_view_local(quo_xpm_t *xc,
                   QUO_xpm_view_t *rview);

int
QUO_xpm_view_by_qid(QUO_xpm_context xpm,
                    int qid,
                    QUO_xpm_view_t *rview);

#ifdef __cplusplus
}
#endif

#endif
