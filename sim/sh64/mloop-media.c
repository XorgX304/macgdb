/* This file is generated by the genmloop script.  DO NOT EDIT! */

/* Enable switch() support in cgen headers.  */
#define SEM_IN_SWITCH

#define WANT_CPU sh64
#define WANT_CPU_SH64

#include "sim-main.h"
#include "bfd.h"
#include "cgen-mem.h"
#include "cgen-ops.h"
#include "sim-assert.h"

/* Fill in the administrative ARGBUF fields required by all insns,
   virtual and real.  */

static INLINE void
sh64_media_fill_argbuf (const SIM_CPU *cpu, ARGBUF *abuf, const IDESC *idesc,
		    PCADDR pc, int fast_p)
{
#if WITH_SCACHE
  SEM_SET_CODE (abuf, idesc, fast_p);
  ARGBUF_ADDR (abuf) = pc;
#endif
  ARGBUF_IDESC (abuf) = idesc;
}

/* Fill in tracing/profiling fields of an ARGBUF.  */

static INLINE void
sh64_media_fill_argbuf_tp (const SIM_CPU *cpu, ARGBUF *abuf,
		       int trace_p, int profile_p)
{
  ARGBUF_TRACE_P (abuf) = trace_p;
  ARGBUF_PROFILE_P (abuf) = profile_p;
}

#if WITH_SCACHE_PBB

/* Emit the "x-before" handler.
   x-before is emitted before each insn (serial or parallel).
   This is as opposed to x-after which is only emitted at the end of a group
   of parallel insns.  */

static INLINE void
sh64_media_emit_before (SIM_CPU *current_cpu, SCACHE *sc, PCADDR pc, int first_p)
{
  ARGBUF *abuf = &sc[0].argbuf;
  const IDESC *id = & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_BEFORE];

  abuf->fields.before.first_p = first_p;
  sh64_media_fill_argbuf (current_cpu, abuf, id, pc, 0);
  /* no need to set trace_p,profile_p */
}

/* Emit the "x-after" handler.
   x-after is emitted after a serial insn or at the end of a group of
   parallel insns.  */

static INLINE void
sh64_media_emit_after (SIM_CPU *current_cpu, SCACHE *sc, PCADDR pc)
{
  ARGBUF *abuf = &sc[0].argbuf;
  const IDESC *id = & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_AFTER];

  sh64_media_fill_argbuf (current_cpu, abuf, id, pc, 0);
  /* no need to set trace_p,profile_p */
}

#endif /* WITH_SCACHE_PBB */


static INLINE const IDESC *
extract (SIM_CPU *current_cpu, PCADDR pc, CGEN_INSN_INT insn, ARGBUF *abuf,
         int fast_p)
{
  const IDESC *id = sh64_media_decode (current_cpu, pc, insn, insn, abuf);

  sh64_media_fill_argbuf (current_cpu, abuf, id, pc, fast_p);
  if (! fast_p)
    {
      int trace_p = PC_IN_TRACE_RANGE_P (current_cpu, pc);
      int profile_p = PC_IN_PROFILE_RANGE_P (current_cpu, pc);
      sh64_media_fill_argbuf_tp (current_cpu, abuf, trace_p, profile_p);
    }
  return id;
}

static INLINE SEM_PC
execute (SIM_CPU *current_cpu, SCACHE *sc, int fast_p)
{
  SEM_PC vpc;

  if (fast_p)
    {
#if ! WITH_SEM_SWITCH_FAST
#if WITH_SCACHE
      vpc = (*sc->argbuf.semantic.sem_fast) (current_cpu, sc);
#else
      vpc = (*sc->argbuf.semantic.sem_fast) (current_cpu, &sc->argbuf);
#endif
#else
      abort ();
#endif /* WITH_SEM_SWITCH_FAST */
    }
  else
    {
#if ! WITH_SEM_SWITCH_FULL
      ARGBUF *abuf = &sc->argbuf;
      const IDESC *idesc = abuf->idesc;
#if WITH_SCACHE_PBB
      int virtual_p = CGEN_ATTR_VALUE (NULL, idesc->attrs, CGEN_INSN_VIRTUAL);
#else
      int virtual_p = 0;
#endif

      if (! virtual_p)
	{
	  /* FIXME: call x-before */
	  if (ARGBUF_PROFILE_P (abuf))
	    PROFILE_COUNT_INSN (current_cpu, abuf->addr, idesc->num);
	  /* FIXME: Later make cover macros: PROFILE_INSN_{INIT,FINI}.  */
	  if (PROFILE_MODEL_P (current_cpu)
	      && ARGBUF_PROFILE_P (abuf))
	    sh64_media_model_insn_before (current_cpu, 1 /*first_p*/);
	  TRACE_INSN_INIT (current_cpu, abuf, 1);
	  TRACE_INSN (current_cpu, idesc->idata,
		      (const struct argbuf *) abuf, abuf->addr);
	}
#if WITH_SCACHE
      vpc = (*sc->argbuf.semantic.sem_full) (current_cpu, sc);
#else
      vpc = (*sc->argbuf.semantic.sem_full) (current_cpu, abuf);
#endif
      if (! virtual_p)
	{
	  /* FIXME: call x-after */
	  if (PROFILE_MODEL_P (current_cpu)
	      && ARGBUF_PROFILE_P (abuf))
	    {
	      int cycles;

	      cycles = (*idesc->timing->model_fn) (current_cpu, sc);
	      sh64_media_model_insn_after (current_cpu, 1 /*last_p*/, cycles);
	    }
	  TRACE_INSN_FINI (current_cpu, abuf, 1);
	}
#else
      abort ();
#endif /* WITH_SEM_SWITCH_FULL */
    }

  return vpc;
}


/* Record address of cti terminating a pbb.  */
#define SET_CTI_VPC(sc) do { _cti_sc = (sc); } while (0)
/* Record number of [real] insns in pbb.  */
#define SET_INSN_COUNT(n) do { _insn_count = (n); } while (0)

/* Fetch and extract a pseudo-basic-block.
   FAST_P is non-zero if no tracing/profiling/etc. is wanted.  */

INLINE SEM_PC
sh64_media_pbb_begin (SIM_CPU *current_cpu, int FAST_P)
{
  SEM_PC new_vpc;
  PCADDR pc;
  SCACHE *sc;
  int max_insns = CPU_SCACHE_MAX_CHAIN_LENGTH (current_cpu);

  pc = GET_H_PC ();

  new_vpc = scache_lookup_or_alloc (current_cpu, pc, max_insns, &sc);
  if (! new_vpc)
    {
      /* Leading '_' to avoid collision with mainloop.in.  */
      int _insn_count = 0;
      SCACHE *orig_sc = sc;
      SCACHE *_cti_sc = NULL;
      int slice_insns = CPU_MAX_SLICE_INSNS (current_cpu);

      /* First figure out how many instructions to compile.
	 MAX_INSNS is the size of the allocated buffer, which includes space
	 for before/after handlers if they're being used.
	 SLICE_INSNS is the maxinum number of real insns that can be
	 executed.  Zero means "as many as we want".  */
      /* ??? max_insns is serving two incompatible roles.
	 1) Number of slots available in scache buffer.
	 2) Number of real insns to execute.
	 They're incompatible because there are virtual insns emitted too
	 (chain,cti-chain,before,after handlers).  */

      if (slice_insns == 1)
	{
	  /* No need to worry about extra slots required for virtual insns
	     and parallel exec support because MAX_CHAIN_LENGTH is
	     guaranteed to be big enough to execute at least 1 insn!  */
	  max_insns = 1;
	}
      else
	{
	  /* Allow enough slop so that while compiling insns, if max_insns > 0
	     then there's guaranteed to be enough space to emit one real insn.
	     MAX_CHAIN_LENGTH is typically much longer than
	     the normal number of insns between cti's anyway.  */
	  max_insns -= (1 /* one for the trailing chain insn */
			+ (FAST_P
			   ? 0
			   : (1 + MAX_PARALLEL_INSNS) /* before+after */)
			+ (MAX_PARALLEL_INSNS > 1
			   ? (MAX_PARALLEL_INSNS * 2)
			   : 0));

	  /* Account for before/after handlers.  */
	  if (! FAST_P)
	    slice_insns *= 3;

	  if (slice_insns > 0
	      && slice_insns < max_insns)
	    max_insns = slice_insns;
	}

      new_vpc = sc;

      /* SC,PC must be updated to point passed the last entry used.
	 SET_CTI_VPC must be called if pbb is terminated by a cti.
	 SET_INSN_COUNT must be called to record number of real insns in
	 pbb [could be computed by us of course, extra cpu but perhaps
	 negligible enough].  */

/* begin extract-pbb */
{
  const IDESC *idesc;
  int icount = 0;

 while (max_insns > 0)
    {
      USI insn = GETIMEMUSI (current_cpu, pc);

      idesc = extract (current_cpu, pc, insn, &sc->argbuf, FAST_P);
      SEM_SKIP_COMPILE (current_cpu, sc, 1);
      ++sc;
      --max_insns;
      ++icount;
      pc += idesc->length;

      if (IDESC_CTI_P (idesc))
        {
          SET_CTI_VPC (sc - 1);

          if (CGEN_ATTR_VALUE (NULL, idesc->attrs, CGEN_INSN_DELAY_SLOT))
            {
              USI insn = GETIMEMUSI (current_cpu, pc);
	      idesc = extract (current_cpu, pc, insn, &sc->argbuf, FAST_P);

              ++sc;
	      --max_insns;
	      ++icount;
	      pc += idesc->length;
            }
	  break;
        }
    }

 Finish:
  SET_INSN_COUNT (icount);
}
/* end extract-pbb */

      /* The last one is a pseudo-insn to link to the next chain.
	 It is also used to record the insn count for this chain.  */
      {
	const IDESC *id;

	/* Was pbb terminated by a cti?  */
	if (_cti_sc)
	  {
	    id = & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_CTI_CHAIN];
	  }
	else
	  {
	    id = & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_CHAIN];
	  }
	SEM_SET_CODE (&sc->argbuf, id, FAST_P);
	sc->argbuf.idesc = id;
	sc->argbuf.addr = pc;
	sc->argbuf.fields.chain.insn_count = _insn_count;
	sc->argbuf.fields.chain.next = 0;
	sc->argbuf.fields.chain.branch_target = 0;
	++sc;
      }

      /* Update the pointer to the next free entry, may not have used as
	 many entries as was asked for.  */
      CPU_SCACHE_NEXT_FREE (current_cpu) = sc;
      /* Record length of chain if profiling.
	 This includes virtual insns since they count against
	 max_insns too.  */
      if (! FAST_P)
	PROFILE_COUNT_SCACHE_CHAIN_LENGTH (current_cpu, sc - orig_sc);
    }

  return new_vpc;
}

/* Chain to the next block from a non-cti terminated previous block.  */

INLINE SEM_PC
sh64_media_pbb_chain (SIM_CPU *current_cpu, SEM_ARG sem_arg)
{
  ARGBUF *abuf = SEM_ARGBUF (sem_arg);

  PBB_UPDATE_INSN_COUNT (current_cpu, sem_arg);

  SET_H_PC (abuf->addr | 1);

  /* If not running forever, exit back to main loop.  */
  if (CPU_MAX_SLICE_INSNS (current_cpu) != 0
      /* Also exit back to main loop if there's an event.
         Note that if CPU_MAX_SLICE_INSNS != 1, events won't get processed
	 at the "right" time, but then that was what was asked for.
	 There is no silver bullet for simulator engines.
         ??? Clearly this needs a cleaner interface.
	 At present it's just so Ctrl-C works.  */
      || STATE_EVENTS (CPU_STATE (current_cpu))->work_pending)
    CPU_RUNNING_P (current_cpu) = 0;

  /* If chained to next block, go straight to it.  */
  if (abuf->fields.chain.next)
    return abuf->fields.chain.next;
  /* See if next block has already been compiled.  */
  abuf->fields.chain.next = scache_lookup (current_cpu, abuf->addr);
  if (abuf->fields.chain.next)
    return abuf->fields.chain.next;
  /* Nope, so next insn is a virtual insn to invoke the compiler
     (begin a pbb).  */
  return CPU_SCACHE_PBB_BEGIN (current_cpu);
}

/* Chain to the next block from a cti terminated previous block.
   BR_TYPE indicates whether the branch was taken and whether we can cache
   the vpc of the branch target.
   NEW_PC is the target's branch address, and is only valid if
   BR_TYPE != SEM_BRANCH_UNTAKEN.  */

INLINE SEM_PC
sh64_media_pbb_cti_chain (SIM_CPU *current_cpu, SEM_ARG sem_arg,
		     SEM_BRANCH_TYPE br_type, PCADDR new_pc)
{
  SEM_PC *new_vpc_ptr;

  PBB_UPDATE_INSN_COUNT (current_cpu, sem_arg);

  /* If we have switched ISAs, exit back to main loop.
     Set idesc to 0 to cause the engine to point to the right insn table.  */
  if ((new_pc & 1) == 0)
  {
    /* Switch to SHcompact.  */
    CPU_IDESC_SEM_INIT_P (current_cpu) = 0;
    CPU_RUNNING_P (current_cpu) = 0;
  }

  /* If not running forever, exit back to main loop.  */
  if (CPU_MAX_SLICE_INSNS (current_cpu) != 0
      /* Also exit back to main loop if there's an event.
         Note that if CPU_MAX_SLICE_INSNS != 1, events won't get processed
	 at the "right" time, but then that was what was asked for.
	 There is no silver bullet for simulator engines.
         ??? Clearly this needs a cleaner interface.
	 At present it's just so Ctrl-C works.  */
      || STATE_EVENTS (CPU_STATE (current_cpu))->work_pending)
    CPU_RUNNING_P (current_cpu) = 0;

  /* Restart compiler if we branched to an uncacheable address
     (e.g. "j reg").  */
  if (br_type == SEM_BRANCH_UNCACHEABLE)
    {
      SET_H_PC (new_pc);
      return CPU_SCACHE_PBB_BEGIN (current_cpu);
    }

  /* If branch wasn't taken, update the pc and set BR_ADDR_PTR to our
     next chain ptr.  */
  if (br_type == SEM_BRANCH_UNTAKEN)
    {
      ARGBUF *abuf = SEM_ARGBUF (sem_arg);
      new_pc = abuf->addr;
      /* Set bit 0 to stay in SHmedia mode.  */
      SET_H_PC (new_pc | 1);
      new_vpc_ptr = &abuf->fields.chain.next;
    }
  else
    {
      ARGBUF *abuf = SEM_ARGBUF (sem_arg);
      SET_H_PC (new_pc);
      new_vpc_ptr = &abuf->fields.chain.branch_target;
    }

  /* If chained to next block, go straight to it.  */
  if (*new_vpc_ptr)
    return *new_vpc_ptr;
  /* See if next block has already been compiled.  */
  *new_vpc_ptr = scache_lookup (current_cpu, new_pc);
  if (*new_vpc_ptr)
    return *new_vpc_ptr;
  /* Nope, so next insn is a virtual insn to invoke the compiler
     (begin a pbb).  */
  return CPU_SCACHE_PBB_BEGIN (current_cpu);
}

/* x-before handler.
   This is called before each insn.  */

void
sh64_media_pbb_before (SIM_CPU *current_cpu, SCACHE *sc)
{
  SEM_ARG sem_arg = sc;
  const ARGBUF *abuf = SEM_ARGBUF (sem_arg);
  int first_p = abuf->fields.before.first_p;
  const ARGBUF *cur_abuf = SEM_ARGBUF (sc + 1);
  const IDESC *cur_idesc = cur_abuf->idesc;
  PCADDR pc = cur_abuf->addr;

  if (ARGBUF_PROFILE_P (cur_abuf))
    PROFILE_COUNT_INSN (current_cpu, pc, cur_idesc->num);

  /* If this isn't the first insn, finish up the previous one.  */

  if (! first_p)
    {
      if (PROFILE_MODEL_P (current_cpu))
	{
	  const SEM_ARG prev_sem_arg = sc - 1;
	  const ARGBUF *prev_abuf = SEM_ARGBUF (prev_sem_arg);
	  const IDESC *prev_idesc = prev_abuf->idesc;
	  int cycles;

	  /* ??? May want to measure all insns if doing insn tracing.  */
	  if (ARGBUF_PROFILE_P (prev_abuf))
	    {
	      cycles = (*prev_idesc->timing->model_fn) (current_cpu, prev_sem_arg);
	      sh64_media_model_insn_after (current_cpu, 0 /*last_p*/, cycles);
	    }
	}

      TRACE_INSN_FINI (current_cpu, cur_abuf, 0 /*last_p*/);
    }

  /* FIXME: Later make cover macros: PROFILE_INSN_{INIT,FINI}.  */
  if (PROFILE_MODEL_P (current_cpu)
      && ARGBUF_PROFILE_P (cur_abuf))
    sh64_media_model_insn_before (current_cpu, first_p);

  TRACE_INSN_INIT (current_cpu, cur_abuf, first_p);
  TRACE_INSN (current_cpu, cur_idesc->idata, cur_abuf, pc);
}

/* x-after handler.
   This is called after a serial insn or at the end of a group of parallel
   insns.  */

void
sh64_media_pbb_after (SIM_CPU *current_cpu, SCACHE *sc)
{
  SEM_ARG sem_arg = sc;
  const ARGBUF *abuf = SEM_ARGBUF (sem_arg);
  const SEM_ARG prev_sem_arg = sc - 1;
  const ARGBUF *prev_abuf = SEM_ARGBUF (prev_sem_arg);

  /* ??? May want to measure all insns if doing insn tracing.  */
  if (PROFILE_MODEL_P (current_cpu)
      && ARGBUF_PROFILE_P (prev_abuf))
    {
      const IDESC *prev_idesc = prev_abuf->idesc;
      int cycles;

      cycles = (*prev_idesc->timing->model_fn) (current_cpu, prev_sem_arg);
      sh64_media_model_insn_after (current_cpu, 1 /*last_p*/, cycles);
    }
  TRACE_INSN_FINI (current_cpu, prev_abuf, 1 /*last_p*/);
}

#define FAST_P 0

void
sh64_media_engine_run_full (SIM_CPU *current_cpu)
{
  SIM_DESC current_state = CPU_STATE (current_cpu);
  SCACHE *scache = CPU_SCACHE_CACHE (current_cpu);
  /* virtual program counter */
  SEM_PC vpc;
#if WITH_SEM_SWITCH_FULL
  /* For communication between cti's and cti-chain.  */
  SEM_BRANCH_TYPE pbb_br_type;
  PCADDR pbb_br_npc;
#endif


  if (! CPU_IDESC_SEM_INIT_P (current_cpu))
    {
      /* ??? 'twould be nice to move this up a level and only call it once.
	 On the other hand, in the "let's go fast" case the test is only done
	 once per pbb (since we only return to the main loop at the end of
	 a pbb).  And in the "let's run until we're done" case we don't return
	 until the program exits.  */

#if WITH_SEM_SWITCH_FULL
#if defined (__GNUC__)
/* ??? Later maybe paste sem-switch.c in when building mainloop.c.  */
#define DEFINE_LABELS
#include "sem-media-switch.c"
#endif
#else
      sh64_media_sem_init_idesc_table (current_cpu);
#endif

      /* Initialize the "begin (compile) a pbb" virtual insn.  */
      vpc = CPU_SCACHE_PBB_BEGIN (current_cpu);
      SEM_SET_FULL_CODE (SEM_ARGBUF (vpc),
			 & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_BEGIN]);
      vpc->argbuf.idesc = & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_BEGIN];

      CPU_IDESC_SEM_INIT_P (current_cpu) = 1;
    }

  CPU_RUNNING_P (current_cpu) = 1;
  /* ??? In the case where we're returning to the main loop after every
     pbb we don't want to call pbb_begin each time (which hashes on the pc
     and does a table lookup).  A way to speed this up is to save vpc
     between calls.  */
  vpc = sh64_media_pbb_begin (current_cpu, FAST_P);

  do
    {
/* begin full-exec-pbb */
{
#if (! FAST_P && WITH_SEM_SWITCH_FULL) || (FAST_P && WITH_SEM_SWITCH_FAST)
#define DEFINE_SWITCH
#define WITH_ISA_COMPACT
#include "sem-media-switch.c"
#else
  vpc = execute (current_cpu, vpc, FAST_P);
#endif
}
/* end full-exec-pbb */
    }
  while (CPU_RUNNING_P (current_cpu));
}

#undef FAST_P


#define FAST_P 1

void
sh64_media_engine_run_fast (SIM_CPU *current_cpu)
{
  SIM_DESC current_state = CPU_STATE (current_cpu);
  SCACHE *scache = CPU_SCACHE_CACHE (current_cpu);
  /* virtual program counter */
  SEM_PC vpc;
#if WITH_SEM_SWITCH_FAST
  /* For communication between cti's and cti-chain.  */
  SEM_BRANCH_TYPE pbb_br_type;
  PCADDR pbb_br_npc;
#endif


  if (! CPU_IDESC_SEM_INIT_P (current_cpu))
    {
      /* ??? 'twould be nice to move this up a level and only call it once.
	 On the other hand, in the "let's go fast" case the test is only done
	 once per pbb (since we only return to the main loop at the end of
	 a pbb).  And in the "let's run until we're done" case we don't return
	 until the program exits.  */

#if WITH_SEM_SWITCH_FAST
#if defined (__GNUC__)
/* ??? Later maybe paste sem-switch.c in when building mainloop.c.  */
#define DEFINE_LABELS
#include "sem-media-switch.c"
#endif
#else
      sh64_media_semf_init_idesc_table (current_cpu);
#endif

      /* Initialize the "begin (compile) a pbb" virtual insn.  */
      vpc = CPU_SCACHE_PBB_BEGIN (current_cpu);
      SEM_SET_FAST_CODE (SEM_ARGBUF (vpc),
			 & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_BEGIN]);
      vpc->argbuf.idesc = & CPU_IDESC (current_cpu) [SH64_MEDIA_INSN_X_BEGIN];

      CPU_IDESC_SEM_INIT_P (current_cpu) = 1;
    }

  CPU_RUNNING_P (current_cpu) = 1;
  /* ??? In the case where we're returning to the main loop after every
     pbb we don't want to call pbb_begin each time (which hashes on the pc
     and does a table lookup).  A way to speed this up is to save vpc
     between calls.  */
  vpc = sh64_media_pbb_begin (current_cpu, FAST_P);

  do
    {
/* begin fast-exec-pbb */
{
#if (! FAST_P && WITH_SEM_SWITCH_FULL) || (FAST_P && WITH_SEM_SWITCH_FAST)
#define DEFINE_SWITCH
#define WITH_ISA_COMPACT
#include "sem-media-switch.c"
#else
  vpc = execute (current_cpu, vpc, FAST_P);
#endif
}
/* end fast-exec-pbb */
    }
  while (CPU_RUNNING_P (current_cpu));
}

#undef FAST_P

