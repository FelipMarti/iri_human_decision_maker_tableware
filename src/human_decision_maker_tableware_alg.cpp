#include "human_decision_maker_tableware_alg.h"

HumanDecisionMakerTablewareAlgorithm::HumanDecisionMakerTablewareAlgorithm(void)
{
	pthread_mutex_init(&this->access_, NULL);
}

HumanDecisionMakerTablewareAlgorithm::~HumanDecisionMakerTablewareAlgorithm
    (void)
{
	pthread_mutex_destroy(&this->access_);
}

void HumanDecisionMakerTablewareAlgorithm::config_update(Config & new_cfg,
							 uint32_t level)
{
	this->lock();

	// save the current configuration
	this->config_ = new_cfg;

	this->unlock();
}

// HumanDecisionMakerTablewareAlgorithm Public API
