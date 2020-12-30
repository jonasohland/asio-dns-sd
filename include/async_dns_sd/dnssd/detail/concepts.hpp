#pragma once

#ifdef __cpp_concepts
#include <concepts>
#define DNSSD_CONCEPTS
#define DNSSD_CONCEPT(name) name
#define DNSSD_CONCEPT_DEF(x) x
#define DNSSD_REQUIRES(constraint) requires constraint
#else
#define DNSSD_CONCEPT(name) typename
#define DNSSD_CONCEPT_DEF(def)
#define DNSSD_REQUIRES(concept)
#endif