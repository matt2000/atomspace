/*
 * BackwardChainer.h
 *
 * Copyright (C) 2014 Misgana Bayetta
 * Copyright (C) 2015 OpenCog Foundation
 *
 * Author: Misgana Bayetta <misgana.bayetta@gmail.com>  October 2014
 *         William Ma <https://github.com/williampma>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef BACKWARDCHAINER_H_
#define BACKWARDCHAINER_H_

#include <opencog/rule-engine/Rule.h>

#include "Target.h"

class BackwardChainerUTest;

namespace opencog
{
    
typedef std::map<Handle, UnorderedHandleSet> VarMultimap;
typedef std::map<Handle, Handle> VarMap;

/**
 * Backward chaining falls in to two cases
 *
 * 1. Truth value query - Given a target atom whose truth value is not
 *    known and a pool of atoms, find a way to estimate the truth
 *    value of the target Atom, via combining the atoms in the pool
 *    using the inference rule. Eg. The target is "Do people breath"
 *
 *    (InheritanceLink people breath)
 *
 *    the truth value of the target is estimated via doing the
 *    inference "People are animals, animals breathe, therefore people
 *    breathe."
 *
 * 2. Variable fulfillment query - Given a target Link (Atoms may be
 *    Nodes or Links) with one or more VariableAtoms among its
 *    targets, figure what atoms may be put in place of these
 *    VariableAtoms, so as to give the grounded targets a high
 *
 *    strength * confidence
 *
 *    Eg. What breathes (InheritanceLink $X breath) can be fulfilled
 *    by pattern matching, whenever there are are multiple values to
 *    fill $X we will use fitness value measure to choose the best
 *    other compound example is what breathes and adds ANDLink
 *    InheritanceLink $X Breath InheritanceLink $X adds
 *
 * Anatomy of a single inferene
 * ============================
 * A single inference step may be viewed as follows
 *
 * 1. Choose inference Rule R and a tuple of Atoms that collectively
 *    match the input condition of the rule
 *
 * 2. Apply choosen rule R to the chosen input Atoms
 *
 * 3. Create an ExecutionLink recording the output found
 *
 * 4. In addition to retaining this ExecutionLink in the Atomspace,
 *    also save the copy of it in the InferenceRepository (this is not
 *    needed for the very first implementation, but will be very
 *    useful once PLN is in regular use.)
 *
 * Anatomy of current implementation
 * =================================
 *
 * 1. First check if the target matches to something in the knowledge base
 *    already
 *
 * 2. If not, choose an inference Rule R (using some Rule selection criteria)
 *    whose output can unify to the target
 *
 * 3. Reverse ground R's input to restrict the permises search
 *
 * 4. Find all permises that matches the restricted R's input by Pattern
 *    Matching
 *
 * 5. For each set of permies, Forward Chain (a.k.a apply the rule, or
 *    Pattern Matching) on the R to see if it can solve the target.
 *
 * 6. If not, add the permises to the targets list (in addition to some
 *    permise selection criteria)
 *
 * 7. Do target selection and repeat.
 */

class BackwardChainer
{
    friend class ::BackwardChainerUTest;

public:
	BackwardChainer(AtomSpace* as, std::vector<Rule>);
	~BackwardChainer();

	void set_target(Handle init_target);

	void do_until(uint max_steps);
	void do_step();

	const VarMultimap& get_chaining_result();

private:

	void process_target(Target& target);

	std::vector<Rule> filter_rules(Handle htarget);
	Rule select_rule(Target& target, const std::vector<Rule>& rules);

	HandleSeq match_knowledge_base(const Handle& htarget,
	                               Handle htarget_vardecl,
	                               bool check_history,
	                               std::vector<VarMap>& vmap);
	HandleSeq ground_premises(const Handle& htarget, const VarMap& vmap, std::vector<VarMap>& vmap_list);
	bool unify(const Handle& hsource, const Handle& hmatch,
	           Handle hsource_vardecl, VarMap& result);

	Handle gen_sub_varlist(const Handle& parent_varlist,
	                       std::set<Handle> varset);

	AtomSpace* _as;
	AtomSpace* _garbage_superspace;
	Handle _init_target;

	// a map of a target, to a map of its variables mapping
	// XXX TODO add a new kind of history that stores what actually happened on
	// on each step, creating an inference tree?
	map<Handle, VarMultimap> _inference_history;

	// XXX TODO add information to each target stating what rules were applied
	// and how often the target was chosen?
	TargetSet _targets_set;
	std::vector<Rule> _rules_set;

	// XXX any additional link should be reflected
	unordered_set<Type> _logical_link_types = { AND_LINK, OR_LINK, NOT_LINK };

};


} // namespace opencog

#endif /* BACKWARDCHAINER_H_ */
