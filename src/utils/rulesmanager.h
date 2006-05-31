// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/05/31 14:11:19  hockm0bm
 * * IRuleMatcher abstract class
 * * RulesManager generic implementation
 *
 *
 */
#ifndef _RULESMANAGER_H_
#define _RULESMANAGER_H_

#include<vector>

namespace rulesmanager
{
	
/** Class for rule matching.
 * 
 * Provides interface for generic rules comparing. Rule is specified as template
 * parameter RuleType. Implementation specific match algorithm should be
 * implemented in functional operator defined by implementator. 
 * <br>
 * When multiple matches can occure (multiple rules matche one original), 
 * priority should be compared and one with the highest (according betterMatch
 * method) should be used.
 */
template<typename RuleType>
class IRuleMatcher
{
public:
	/** Type for matching priority.
	 * Use betterMatch method for priority comparing.
	 */
	typedef int priority_t;

	/** Checks whether prio2 is higher than prio1.
	 * @param prio1 Priority.
	 * @param prio2 Priority.
	 * 
	 * @return true if prio2 is at least prio1.
	 */
	virtual bool betterMatch(const priority_t & prio1, const priority_t & prio2)const
	{
		return prio1<=prio2;
	}

	/** Functor to compare given rules.
	 * @param original Rule from configuration.
	 * @param rule Rule to check.
	 * @param priority Place where to set priority if given rule matches (if
	 * NULL, nothing is set).
	 *
	 * Implementator should define when given rules matches and formulate
	 * priorities if multiple rules can match original. Match with higher
	 * priority should have higher precedence (use betterMatch method for
	 * priority comparing).
	 * <br>
	 * If given rule matches and priority parameter is non NULL, sets priority
	 * to place where it points.
	 * 
	 * @return true if given rule matches given original, false otherwise.
	 */
	virtual bool operator()(const RuleType & original, const RuleType & rule, priority_t * priority)const =0;
};

/** Manager for rules.
 *
 * Template class to maintain mapping from rules to rules targets (specified by
 * template parameters RuleType and RuleTarget) and query for RuleTargets from
 * given rules with most specific matching one.
 * <br>
 * RuleType template parameter specifies rule structure. It has to implement at
 * least bool equals(const RuleType & )const method which returns true for two 
 * same rules (Note same are allways mathich but oposit implication doesn't 
 * have to be true). 
 * <br>
 * RuleTarget template parameter specifies target of the rule. There are no
 * constrains (except for correct allocation, deallocation and copying) on type.
 * RulesManager user has to understand usage and meaning.
 * <br>
 * Uses IRuleMatcher implementation for specific rule decisions, defined in
 * constructor. This can be changed in runtime to better fit different
 * strathegies. 
 * <br>
 * This class is responsible for correct rules storing and mapping to rule
 * targets. All matching logic is delegated to the IRuleMatcher and just best
 * candidate is returned. Mapping is stored in container defined as RuleStorage
 * template paramter. This has to provide: forward iterators (both const and non
 * const), clear method, push_back method, erase method.
 */
template<typename RuleType, 
        typename RuleTarget, 
        typename RuleStorage=std::vector<std::pair<RuleType,RuleTarget> > >
class RulesManager
{
public:
	typedef IRuleMatcher<RuleType> RuleMatcherType;
	typedef typename RuleStorage::iterator iterator;
	typedef typename RuleStorage::const_iterator const_iterator;
	typedef typename RuleStorage::value_type MappingType;
private:
	
	/** Rules to targets mapping.
	 */
	RuleStorage mapping;

	/** IRuleMatcher implementator for rule matching.
	 *
	 * First implementation is set in constructor and can be changed by
	 * setRuleMatcher method. If NULL is provided, findMatching always
	 * return false.
	 */
	const RuleMatcherType * matcher;
	
	/** Searches for given rule in [start, end) range.
	 * @param rule Rule to search.
	 * @param start Iterator to first element to search.
	 * @param end Iterator behind last element to consider.
	 *
	 * Checks for all elements in range [start, end) and compares it with given
	 * rule (using RuleType::equals method). If found, returns iterator.
	 *
	 * @return iterator to element which has same rule or end iterator if not
	 * found.
	 */
	iterator find(RuleType & rule, iterator & start, iterator & end)
	{
		for(iterator i=start; i!=end; i++)
		{
			RuleType original=i->first;
			// rule and original must be same
			if(original.equals(rule))
				return i;
		}

		return end;
	}
public:
	/** Empty constructor.
	 * Initializes matcher to NULL.
	 */
	RulesManager():matcher(NULL){};
	
	/** Constructor.
	 * @param m Implementation of rules matcher.
	 *
	 * Sets matcher field and creates empty mapping.
	 */
	RulesManager(const RuleMatcherType * m):matcher(m){};

	/** Constructor with initial mapping.
	 * @param m Implementation of rules matcher.
	 * @param map Initial rules mapping.
	 *
	 * Sets matcher field and initializes mapping with given storage.
	 */
	RulesManager(const RuleMatcherType * m, RuleStorage map):mapping(map.begin(), map.end()), matcher(m){};

	/** Virtual destructor.
	 * This method is empty, because no special treatment is required.
	 */
	virtual ~RulesManager(){};
	
	/** Sets new rule matcher implementation.
	 * @param newMatcher New matcher implementation.
	 *
	 * @return Old matcher implmentation.
	 */
	virtual const RuleMatcherType * setRuleMatcher(const RuleMatcherType * newMatcher)
	{
		const RuleMatcherType * old=matcher;

		matcher=newMatcher;
		return old;
	}
	
	/** Adds given rule, target mapping.
	 * @param ruleDef Rule definition.
	 * @param target Target for given rule.
	 *
	 * Inserts mapping with given ruleDef, target pair. Mapping is inserted even
	 * if ruleDef is already in. It depends on implementation of IRuleMatcher
	 * which one is used (by default one specified later because
	 * IRuleMatcher::betterMatch returns true for same priority by default).
	 */
	virtual void addRule(RuleType ruleDef, RuleTarget target)
	{
		mapping.push_back(MappingType(ruleDef, target));
	}

	/** Removes mapping for given ruleDef.
	 * @param ruleDef Rule definition.
	 * @param target Place where to place associated target.
	 *
	 * First occurence in mapping is removed. 
	 * <br>
	 * If mapping is found and target parameter is non NULL, rule target is set
	 * to place where it points.
	 *
	 * @return true if rule was removed, false otherwise.
	 */
	virtual bool delRule(RuleType ruleDef, RuleTarget * target)
	{
		iterator i=find(ruleDef, mapping.begin(), mapping.end());
		if(i==mapping.end())
			return false;

		// elemtent is found, we can remove it
		// if target is not null, stores found target
		if(target)
			*target=i->second;
		mapping.erase(i);
		return true;
	}

	/** Removes all mappings matching given rule.
	 * @param ruleDef Rule to match.
	 * @param removed RuleStorage with all removed rule, target pairs (only if
	 * non NULL).
	 *
	 * Goes through whole mapping and checks all rules whether they match given
	 * rule. If yes, removes them from mapping and if removed parameter is non
	 * NULL, inserts rule, target pair to it (container is cleared at the
	 * begining in any case).
	 * <br>
	 * If matcher is not specified (it is NULL), returns immediately after
	 * removed container is cleared.
	 */
	virtual void delMatching(RuleType ruleDef, RuleStorage * removed)
	{
		// clears given container if non NULL
		if(removed)
			removed->clear();
		
		// checks for IRuleMatcher
		if(!matcher)
			return;
		iterator i=mapping.begin();
		for(; i!=mapping.end(); )
		{
			RuleType original=i->first;
			if((*matcher)(original, ruleDef))
			{
				// this element matches
				if(removed)
					// stores current mapping
					removed->push_back(RuleStorage::value_type(i->first, i->second));
				// removes element and sets iterator to one immediately after
				// member
				i=mapping.erase(i);
				continue;
			}

			// doesn't match, simply moves iterator
			i++;
		}
	}

	/** Clears whole mapping.
	 * @param removed Place where to store removed mapping (only if non NULL).
	 *
	 * If removed parameter is non NULL, copies mapping content to it befor
	 * clearing (given container is cleared before).
	 */
	virtual void clear(RuleStorage * removed)
	{
		if(removed)
		{
			// cleares and copies
			removed.clear();
			removed.insert(mapping.begin(), mapping.end());
		}

		mapping.clear();
	}

	/** Finds best fit mapping.
	 * @param rule Rule to find.
	 * @param target Place where to put target if rule matches (only if non
	 * NULL).
	 * 
	 * Goes through whole mapping and compares given rule with those from
	 * mapping using IRuleMatcher implementation. If multiple rules matches use
	 * one which matches the best according priority.
	 * <br>
	 * If at least one mapping matches and target parameter is non NULL,
	 * associated target is stored to place where it points.
	 * <br>
	 * if no matcher is specified (it is NULL), immediately returns with false.
	 * 
	 * @return true if rule matches at least one from mapping, false otherwise.
	 */
	virtual bool findMatching(const RuleType & rule, RuleTarget * target)const
	{
		typename RuleMatcherType::priority_t topPrio;
		const_iterator topPos=mapping.end();

		for(const_iterator i=mapping.begin(); i!=mapping.end(); i++)
		{
			MappingType element=*i;
			const RuleType original=element.first;
			typename RuleMatcherType::priority_t prio;
			if((*matcher)(original, rule, &prio))
			{
				// rule matches with current one	
				if(topPos==mapping.end())
				{
					// first one which matches
					topPos=i;
					topPrio=prio;
					continue;
				}

				// we have already found matching, so compares priorities and
				// uses just betterMatch
				if(matcher->betterMatch(topPrio, prio))
				{
					topPos=i;
					topPrio=prio;
					continue;
				}
			}
		}

		// at least one matches if topPos is set to inner iterator
		if(topPos!=mapping.end())
		{
			if(target)
			{
				// if target is non NULL, sets target value
				MappingType foundElement=*topPos;
				*target=foundElement.second;
			}
			return true;
		}
		
		// nothing matched
		return false;
	}
};

} // namespace rulesmanager

#endif