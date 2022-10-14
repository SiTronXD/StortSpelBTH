local script = {}

enum ActiveAttack { noActive, lightActive, heavyActive, comboActive }

function script:init()
	print("init with ID: " .. self.ID)

	self.health = 100
	
	self.lightHit = 15
	self.heavyHit = 20

	self.comboLightHit = 30
	self.comboMixHit = 40
	self.comboHeavyHit = 50

	self.lightAttackTime = 3
	self.heavyAttackTime = 5
	self.comboAttackTime = 7

	self.comboOrder = ""
	self.combos = { "Light Light ", "Light Heavy Light ", "Heavy Light Heavy " }

	self.timer = 0
	self.hitTimer = 0

	ActiveAttack activeAttack = noActive;

	Scene* scene;
	bool gotHit = true;

end

function script:update(dt)
	if (Input::isMouseButtonPressed(Mouse::LEFT) && gotHit == true)
	{
		lightAttack(self, gotHit);
	}
	else if (Input::isMouseButtonPressed(Mouse::RIGHT) && gotHit == true)
	{
		heavyAttack(self, gotHit);
	}
end

float& getHealth(self& self)
{
	return self.health;
};

function script.checkActiveAttack()
	self.hitTimer = std::chrono::system_clock::now() - self.timer;

	switch (self.activeAttack)
	{
	If (self.activeAttack.lightActive)
	then
		--If it takes too long between attacks, resets combo.
		if (self.hitTimer.count() > self.lightAttackTime + 2)
		then
			self.activeAttack = self.activeAttack.noActive
			self.comboOrder.clear()
		elseif (self.hitTimer > self.lightAttackTime)
		then
			self.activeAttack = self.activeAttack.noActive
		end
	If (self.activeAttack.heavyActive)
		-- If it takes too long between attacks, resets combo.
		if (self.hitTimer > self.heavyAttackTime + 2)
		then
			self.activeAttack = self.activeAttack.noActive
			self.comboOrder.clear()
		elseif (self.hitTimer > self.heavyAttackTime)
		then
			self.activeAttack = self.activeAttack.noActive
		end
	If (self.activeAttack.comboActive)
		if (self.hitTimer > self.comboAttackTime)
		then
			self.activeAttack = self.activeAttack.noActive
		end
end

	bool lightAttack(self& self, bool gotHit)
	{
		checkActiveAttack(self);

		if (self.activeAttack == noActive)
		{
			self.hitTimer = std::chrono::duration<float>(self.lightAttackTime);
			self.timer = std::chrono::system_clock::now();
			self.comboOrder.append("Light ");

			// If combo is lhh there can be no combo, combo is reset.
			if (self.comboOrder == "Light Heavy Heavy ")
			{
				self.comboOrder.clear();
			}

			self.activeAttack = lightActive;

			if (gotHit)
			{
				if (checkCombo(self))
				{
					return true;
				}
				else
				{
					self.health -= self.lightHit;
					//std::cout << self.comboOrder + "\nYou hit the enemy! New health is " + std::to_string(self.health) + "\n\n";
					return true;
				}
			}
		}
		return false;
	};
	bool heavyAttack(self& self, bool gotHit)
	{
		checkActiveAttack(self);

		if (self.activeAttack == noActive)
		{
			self.hitTimer = std::chrono::duration<float>(self.heavyAttackTime);
			self.timer = std::chrono::system_clock::now();
			self.comboOrder.append("Heavy ");

			// If combo starts with Heavy Heavy , there can be no combo, combo is reset.
			if (self.comboOrder == "Heavy Heavy ") { self.comboOrder.clear(); }

			self.activeAttack = heavyActive;

			if (gotHit)
			{
				if (checkCombo(self)) { return true; }
				else
				{
					self.health -= self.heavyHit;
					//std::cout << self.comboOrder + "\nYou hit the enemy! New health is " + std::to_string(self.health) + "\n\n";
					return true;
				}
			}
		}

		return false;
	};

	// Executes combo attack.
	void comboAttack(self& self, int idx)
	{
		if (idx == 0)
		{
			self.health -= self.comboLightHit;
			//std::cout << "You hit the enemy with a light combo! New health is " + std::to_string(self.health) + "\n\n";
			self.comboOrder.clear();
			self.activeAttack = comboActive;
		}
		else if (idx == 1)
		{
			self.health -= self.comboMixHit;
			//std::cout << "You hit the enemy with a mix combo! New health is " + std::to_string(self.health) + "\n\n";
			self.comboOrder.clear();
			self.activeAttack = comboActive;
		}
		else if (idx == 2)
		{
			self.health -= self.comboHeavyHit;
			//std::cout << "You hit the enemy with a heavy combo! New health is " + std::to_string(self.health) + "\n\n";
			self.comboOrder.clear();
			self.activeAttack = comboActive;
		}
	};

	// Check if there is a combo and if it is, execute the combo.
	bool checkCombo(self& self)
	{
		for (int i = 0; i < self.combos.size(); i++)
		{
			if (self.comboOrder.c_str() == self.combos[i])
			{
				comboAttack(self, i);
				return true;
			}
		}
		return false;
	};

};

return script