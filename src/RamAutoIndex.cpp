/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file RamAutoIndex.cpp
 *
 * Implements class for auto index generation of Ram index programs.
 * The minimal indexes are generated by solving a maximum matching
 * problem.
 ***********************************************************************/

#include "RamAutoIndex.h"

#include <cstdlib>
#include <string>

namespace souffle {

/*
 * Class IndexSet
 */

/** map the keys in the key set to lexicographical order */
void RamAutoIndex::solve() {
    if (searches.empty()) {
        return;
    }

    // check whether one of the naive indexers should be used
    static const char ENV_NAIVE_INDEX[] = "SOUFFLE_USE_NAIVE_INDEX";
    if (std::getenv(ENV_NAIVE_INDEX)) {
        static bool first = true;

        // print a warning - only the first time
        if (first) {
            std::cout << "WARNING: auto index selection disabled, naive indexes are utilized!!\n";
            first = false;
        }

        // every search pattern gets its naive index
        for (SearchColumns cur : searches) {
            // obtain order
            LexicographicalOrder order;
            SearchColumns mask = cur;
            for (int i = 0; mask != 0; i++) {
                if (!(1 << i & mask)) {
                    continue;
                }
                order.push_back(i);
                // clear bit
                mask &= ~(1 << i);
            }

            // add new order
            orders.push_back(order);

            // register pseudo chain
            chainToOrder.push_back(Chain());
            chainToOrder.back().insert(cur);
        }

        //		std::cout << "Orders: " << orders << "\n";
        //		std::cout << "Chains: " << chainToOrder << "\n";

        return;
    }

    // Construct the matching poblem
    for (SearchSet::const_iterator it = searches.begin(); it != searches.end(); ++it) {
        // For this node check if other nodes are strict subsets
        for (SearchSet::const_iterator itt = searches.begin(); itt != searches.end(); ++itt) {
            if (isStrictSubset(*it, *itt)) {
                matching.addEdge(*it, toB(*itt));
            }
        }
    }

    // Perform the hopcroft-karp on the graph and receive matchings (mapped A->B and B->A)
    // Assume: alg.calculate is not called on an empty graph
    ASSERT(!searches.empty());
    const RamMaxMatching::Matchings& matchings = matching.calculate();

    // Extract the chains given the nodes and matchings
    const ChainOrderMap chains = getChainsFromMatching(matchings, searches);

    // Should never get no chains back as we never call calculate on an empty graph
    ASSERT(!chains.empty());

    for (ChainOrderMap::const_iterator it = chains.begin(); it != chains.end(); ++it) {
        std::vector<int> ids;
        SearchColumns initDelta = *(it->begin());
        insertIndex(ids, initDelta);

        for (Chain::iterator iit = it->begin(); next(iit) != it->end(); ++iit) {
            SearchColumns delta = *(next(iit)) - *iit;
            insertIndex(ids, delta);
        }

        ASSERT(!ids.empty());

        orders.push_back(ids);
    }

    // Construct the matching poblem
    for (SearchSet::const_iterator it = searches.begin(); it != searches.end(); ++it) {
        int idx = map(*it);
        size_t l = card(*it);
        SearchColumns k = 0;
        for (size_t i = 0; i < l; i++) {
            k = k + (1 << (orders[idx][i]));
        }
        ASSERT(k == *it && "incorrect lexicographical order");
    }
}

/** given an unmapped node from set A we follow it from set B until it cannot be matched from B
  if not mateched from B then umn is a chain*/
RamAutoIndex::Chain RamAutoIndex::getChain(const SearchColumns umn, const RamMaxMatching::Matchings& match) {
    SearchColumns start = umn;  // start at an unmateched node
    Chain chain;
    // Assume : no circular mappings, i.e. a in A -> b in B -> ........ -> a in A is not allowed.
    // Given this, the loop will terminate
    while (true) {
        RamMaxMatching::Matchings::const_iterator mit = match.find(toB(start));  // we start from B side
        chain.insert(start);

        if (mit == match.end()) {
            return chain;
        }

        SearchColumns a = mit->second;
        chain.insert(a);
        start = a;
    }
}

/** get all chains from the matching */
const RamAutoIndex::ChainOrderMap RamAutoIndex::getChainsFromMatching(
        const RamMaxMatching::Matchings& match, const SearchSet& nodes) {
    ASSERT(!nodes.empty());

    // Get all unmatched nodes from A
    const SearchSet& umKeys = getUnmatchedKeys(match, nodes);

    // Case: if no unmatched nodes then we have an anti-chain
    if (umKeys.empty()) {
        for (SearchSet::const_iterator nit = nodes.begin(); nit != nodes.end(); ++nit) {
            std::set<SearchColumns> a;
            a.insert(*nit);
            chainToOrder.push_back(a);
            return chainToOrder;
        }
    }

    ASSERT(!umKeys.empty());

    // A worklist of used nodes
    SearchSet usedKeys;

    // Case: nodes < umKeys or if nodes == umKeys then anti chain - this is handled by this loop
    for (SearchSet::iterator it = umKeys.begin(); it != umKeys.end(); ++it) {
        Chain c = getChain(*it, match);
        ASSERT(!c.empty());
        chainToOrder.push_back(c);
    }

    ASSERT(!chainToOrder.empty());

    return chainToOrder;
}

}  // end of namespace souffle
