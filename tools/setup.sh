#!/bin/bash

# ==============================================================================
# TentacleOS Developer Environment Setup
# ==============================================================================
# Run this once after cloning the repository.

GREEN='\033[0;32m'
NC='\033[0m'

echo "Setting up TentacleOS development environment..."

git config core.hooksPath .githooks
echo -e "  Git hooks path set to ${GREEN}.githooks/${NC}"

echo -e "${GREEN}Setup complete!${NC}"
