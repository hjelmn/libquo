/**
 * Copyright (c) 2013      Los Alamos National Security, LLC.
 *                         All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "quo.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "mpi.h"

/**
 * this code tests the QUO_dist_work_member routine.
 */

typedef struct info_t {
    QUO_t *q;
    int rank;
    int nranks;
    int noderank;
    int nnoderanks;
    int ncores;
    QUO_obj_type_t tres;
} info_t;

typedef void (*bp)(info_t *q);

typedef struct bind_t {
    /* bind name */
    char *bname;
    /* bind function pointer */
    bp bfp;
    /* pop function pointer */
    bp pfp;
} bind_t;

static void
emit_bind(info_t *i)
{
    char *binds = NULL;
    assert(QUO_SUCCESS == QUO_stringify_cbind(i->q, &binds));
    printf("bbb %d %s\n", i->rank, binds);
    free(binds);
}

static void
tsync(const info_t *i)
{
    MPI_Barrier(MPI_COMM_WORLD);
    usleep((i->rank) * 1000);
}

static void
popbind(info_t *i)
{
    assert(QUO_SUCCESS == QUO_bind_pop(i->q));
}

static void
nobind(info_t *i)
{
    assert(QUO_SUCCESS == QUO_bind_push(i->q, QUO_BIND_PUSH_PROVIDED,
                                        QUO_OBJ_MACHINE, 0));
}

static void
tightbind(info_t *i)
{
    assert(QUO_SUCCESS == QUO_bind_push(i->q, QUO_BIND_PUSH_PROVIDED,
                                        QUO_OBJ_CORE, i->noderank));
}

static void
insanity(info_t *i)
{
    if (0 == i->noderank || 4 == i->noderank || 5 == i->noderank || 6 == i->noderank) {
        assert(QUO_SUCCESS == QUO_bind_push(i->q, QUO_BIND_PUSH_OBJ,
                                            QUO_OBJ_MACHINE, -1));
    }
    else {
        assert(QUO_SUCCESS == QUO_bind_push(i->q, QUO_BIND_PUSH_PROVIDED,
                                            QUO_OBJ_CORE, i->noderank));
    }
}

static bind_t binds[] =
{
    {"complete set overlap", nobind, popbind},
    {"no set overlap", tightbind, popbind},
    {"some set overlap", insanity, popbind},
    {NULL, NULL}
};

int
main(int argc, char **argv)
{
    info_t info;
    int work_member = 0, max_members_per_res = 10;
    int nres = 0, rc = EXIT_SUCCESS;
    info.tres = QUO_OBJ_NODE;

    assert(MPI_SUCCESS == MPI_Init(&argc, &argv));
    assert(QUO_SUCCESS == QUO_construct(&info.q));
    assert(QUO_SUCCESS == QUO_init(info.q));
    assert(MPI_SUCCESS == MPI_Comm_size(MPI_COMM_WORLD, &info.nranks));
    assert(MPI_SUCCESS == MPI_Comm_rank(MPI_COMM_WORLD, &info.rank));
    assert(QUO_SUCCESS == QUO_noderank(info.q, &info.noderank));
    assert(QUO_SUCCESS == QUO_nnoderanks(info.q, &info.nnoderanks));
    assert(QUO_SUCCESS == QUO_nnumanodes(info.q, &nres));
    assert(QUO_SUCCESS == QUO_ncores(info.q, &info.ncores));
    setbuf(stdout, NULL);
    if (info.ncores < info.nnoderanks) {
        if (0 == info.noderank) {
            fprintf(stderr, "xxx cannot continue: %d core(s) < %d rank(s).\n",
                    info.ncores, info.nranks);
        }
        rc = EXIT_FAILURE;
        goto done;
    }
    if (0 == nres) {
        assert(QUO_SUCCESS == QUO_nsockets(info.q, &nres));
        info.tres = QUO_OBJ_SOCKET;
        if (0 == nres) {
            fprintf(stderr, "xxx cannot continue with test! xxx\n");
            rc = EXIT_FAILURE;
            goto done;
        }
    }
    if (0 == info.rank) {
        printf("ooo starting test: max mems per %d = %d (see quo.h) ooo\n",
               (int)info.tres, max_members_per_res);
    }
    for (int i = 0; NULL != binds[i].bname; ++i) {
        tsync(&info);
        if (0 == info.rank) printf("--- %s\n", binds[i].bname);
        binds[i].bfp(&info);
        emit_bind(&info);
        tsync(&info);
        assert(QUO_SUCCESS == QUO_dist_work_member(info.q, info.tres,
                                                   max_members_per_res,
                                                   &work_member));
        printf("*** rank %d work member: %d\n", info.rank, work_member);
        tsync(&info);
        binds[i].pfp(&info);
        tsync(&info);
    }
done:
    assert(QUO_SUCCESS == QUO_finalize(info.q));
    assert(QUO_SUCCESS == QUO_destruct(info.q));
    assert(MPI_SUCCESS == MPI_Finalize());
    return rc;
}
