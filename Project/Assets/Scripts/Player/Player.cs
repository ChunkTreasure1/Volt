using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Threading;
using Volt;

namespace Project
{
    public class Player : Character
    {
        //Public: ----------------------------------------------
        public PlayerInputHandler playerEventHandler;
        public Entity FPSCamera
        {
            get
            {
                return myFirstPersonCamera;
            }
        }
        public bool IsAiming;

        public bool IsRunning
        {
            get { return myIsRunning; }
        }

        public bool IsTargetable
        {
            get
            {
                return myIsTargetable;
            }

            set
            {
                myIsTargetable = value;
            }
        }

        public AnimationControllerComponent CharacterAnimationController
        {
            get
            {
                return myAnimationController;
            }
        }

        //Private: ----------------------------------------------
        private Entity myFirstPersonCamera;
        private CharacterControllerComponent myCharacterController;
        private AnimationControllerComponent myAnimationController;

        [RepContinuous]
        private float myJumpForce;

        private bool myShouldRegen;
        private EntityTimer myRegenTimer;

        private bool myIsTargetable;
        private bool myIsActivePlayer;
        private bool myIsRunning;
        private bool myWasRunning;
        private bool myOutOfBreath;
        private float myRunCooldown;
        private float myStamina;
        private float myCurrentMaxSpeed;
        private float myCurrentAcceleration;
        private bool myIsMoving;
        private Vector3 myMoveDirection;
        private Vector3 myVelocity;
        private bool myIsDead;
        private bool myIsPaused;

        private uint myCurrentWeapon = 0;
        private Entity[] myWeapons;
        public PlayerStats myStats;
        private List<Perk_Base> myPerks = new List<Perk_Base>();
        private bool myHasShotWithNonAutomatic;
        private float myMeleeRange;
        private float myMeleeDamage;
        private bool myIsMeleeing;
        private float myMeleeTime = 0.3f;
        private float myMeleeTimer;
        private bool myIsUnequipping;
        private float myUnequipTime = 0.7f;
        private float myUnequipTimer;

        [RepNotify("LogN")]
        public float myPlayerTestNotificationVariable = 0.0f;

        [RepContinuous("SetModelRotation")]
        public float myCharRotY = 0.0f;

        //AUDIO
        private AudioPlayerHandler myAudioHandler;

        // Debug
        public bool animGraphShooting;
        public bool animGraphWalking;
        public bool animGraphRunning;
        public bool animGraphAiming;
        public bool animGraphMeleeing;
        public bool animGraphReloading;
        public bool animGraphEquipping;
        public bool animGraphUnequipping;
        public int animGraphWeaponID;

        // TESTING
        public Font font;

        public void SetModelRotation()
        {
            if (myAnimationController == null) return;
            if (entity.HasScript<PlayerInputHandler>()) return;
            //myAnimationController.entity.rotation = new Quaternion(0, myCharRotY, 0, 0);
        }

        public void LogC()
        {
            Volt.Log.Info("con works");
        }
        public void LogN()
        {
            Volt.Log.Info("notify works");
        }
        private void OnCreate()
        {
            myIsTargetable = true;
            myIsActivePlayer = false;
            if (entity.HasScript<PlayerInputHandler>())
            {
                playerEventHandler = entity.GetScript<PlayerInputHandler>();
                playerEventHandler.OnPlayerInput += OnPlayerInputEvent;
                myIsActivePlayer = true;
            }

            myCharacterController = entity.GetComponent<CharacterControllerComponent>();
            myFirstPersonCamera = entity.FindChild("FirstPersonCamera");
            myAnimationController = myFirstPersonCamera.FindChild("PlayerMesh").GetComponent<AnimationControllerComponent>();
            myStats = new PlayerStats();

            //READ PLAYER DATA FILE
            string readText = File.ReadAllText(ProjectManager.GetDirectory() + "/Assets/Data/PlayerData.txt");
            myStats.Initialize((PlayerStats.PlayerType)int.Parse(readText));

            Log.Info(myStats.myPlayerType.ToString());

            // Movement
            myCurrentMaxSpeed = myStats.WalkMovementSpeed;
            myCurrentAcceleration = myStats.MovementAcceleration;
            myStamina = myStats.Stamina;
            myRunCooldown = myStats.RunCooldown;
            myShouldRegen = false;
            myOutOfBreath = false;
            myIsRunning = false;
            myWasRunning = false;
            myIsMoving = false;
            IsAiming = false;
            myVelocity = Vector3.Zero;
            myMeleeRange = 150.0f;
            myMeleeDamage = 150.0f;

            // Jump
            myCharacterController.gravity *= myStats.GravityMultiplier;
            myJumpForce = Mathf.Sqrt(2 * -Physics.gravity.y * myStats.GravityMultiplier * myStats.JumpHeight);

            if (myFirstPersonCamera != null)
            {
                Vision.SetCameraFollow(myFirstPersonCamera, entity);
            }

            // Weapons
            myCurrentWeapon = 0;
            myWeapons = new Entity[2];

            foreach (Entity child in entity.children)
            {
                if (child.HasScript<Weapon>())
                {

                    if (myWeapons[0] == null)
                    {
                        myWeapons[0] = child;
                        child.GetScript<Weapon>().Behaviour = WeaponManager.Instance.CreateWeapon(WeaponId.M1911);
                        child.GetScript<Weapon>().Behaviour.Information.MagazineAmmo = 32;
                    }
                    else
                    {
                        myWeapons[1] = child;
                        child.GetScript<Weapon>().Behaviour = WeaponManager.Instance.CreateWeapon(WeaponId.Spas12);
                        //child.GetScript<Weapon>().Behaviour.IsPaP = true;
                    }
                }
            }

            myAnimationController?.controller?.SetParameter("WeaponID", (int)myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id);
            Log.Trace(((int)myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id).ToString());

            myHasShotWithNonAutomatic = false;

            myMeleeTimer = myMeleeTime;
            myIsMeleeing = false;

            myUnequipTimer = myUnequipTime;
            myIsUnequipping = false;

            myIsDead = false;

            //CAMERA
            Vision.SetCameraFoV(myFirstPersonCamera, 60);

            //UI
            UIManager.Instance.WeaponInteractionEvent();
            UIManager.Instance.ChangeUIStateEvent += OnPause;

            //AUDIO
            myAudioHandler = entity.FindChild("Audio").GetScript<AudioPlayerHandler>();
        }

        private void OnPlayerInputEvent(List<PlayerInputType> input)
        {
            if (myIsDead || myIsPaused) { return; }
            Weapon(input);
            Movement(input);
        }

        private void OnUpdate(float deltaTime)
        {
            if (myIsDead) { return; }

            // Health regen
            if (myShouldRegen && myStats.CurrentHealth < myStats.ModifiedMaxHealth)
            {
                myStats.CurrentHealth += myStats.HealthRegen * Time.deltaTime;
            }
            else
            {
                myShouldRegen = false;
            }

            if (myFirstPersonCamera != null)
            {
                Vision.SetCameraFollow(myFirstPersonCamera, entity);
            }

            // Debug anim graph
            if (myAnimationController != null)
            {
                animGraphShooting = myAnimationController.controller.GetParameterBool("IsShooting");
                animGraphWalking = myAnimationController.controller.GetParameterBool("IsWalking");
                animGraphRunning = myAnimationController.controller.GetParameterBool("IsRunning");
                animGraphAiming = myAnimationController.controller.GetParameterBool("IsAiming");
                animGraphMeleeing = myAnimationController.controller.GetParameterBool("IsMeleeing");
                animGraphReloading = myAnimationController.controller.GetParameterBool("IsReloading");
                animGraphEquipping = myAnimationController.controller.GetParameterBool("IsEquipping");
                animGraphUnequipping = myAnimationController.controller.GetParameterBool("IsUnequipping");
                animGraphWeaponID = myAnimationController.controller.GetParameterInt("WeaponID");
            }

            myAnimationController?.controller?.SetParameter("IsShooting", false);

            if (myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.IsReloading)
            {
                myAnimationController?.controller?.SetParameter("IsStartingToReload", false);
            } else
            {
                myAnimationController?.controller?.SetParameter("IsReloading", false);
            }

            LandLogic();
        }

        private void Weapon(List<PlayerInputType> input)
        {
            if (myIsUnequipping)
            {
                if (myUnequipTimer > 0)
                {
                    myUnequipTimer -= Time.deltaTime;
                }
                else
                {
                    myIsUnequipping = false;
                    myAnimationController?.controller?.SetParameter("IsUnequipping", false);
                    myAnimationController?.controller?.SetParameter("WeaponID", (int)myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id);
                    myAnimationController?.controller?.SetParameter("IsEquipping", true);
                    entity.CreateTimer(0.5f, () => { myAnimationController?.controller?.SetParameter("IsEquipping", false); });
                    UIManager.Instance.WeaponInteractionEvent();
                    myUnequipTimer = myUnequipTime;
                    myAudioHandler?.Equip();
                }
            }

            if (myIsMeleeing)
            {
                if (myMeleeTimer > 0)
                {
                    myMeleeTimer -= Time.deltaTime;
                }
                else
                {
                    myIsMeleeing = false;
                    myAnimationController?.controller?.SetParameter("IsMeleeing", false);
                    myMeleeTimer = myMeleeTime;
                }
            }

            if (input.Contains(PlayerInputType.Melee) && !myIsMeleeing)
            {
                myIsMeleeing = true;
                myAnimationController?.controller?.SetParameter("IsMeleeing", true);
                uint layerMask = 1 << 7;

                RaycastHit hit;
                if (Physics.Raycast(FPSCamera.position, FPSCamera.forward, out hit, myMeleeRange, layerMask))
                {
                    if (hit.entity.HasScript<ColliderAttachment>())
                    {
                        NetEvents.EventFromLocalId(hit.entity.Id, eNetEvent.Hit, myMeleeDamage, eBodyPart.Other, false);
                        myAudioHandler?.PlayMelee(true);
                    }
                }
                else
                {
                    myAudioHandler?.PlayMelee(false);
                }
            }

            if (input.Contains(PlayerInputType.SwitchToWeaponOne))
            {
                if (myWeapons[0] != null && myWeapons[0]?.GetScript<Weapon>()?.Behaviour.Id != 0 && myCurrentWeapon != 0)
                {
                    myCurrentWeapon = 0;
                    myIsUnequipping = true;
                    myAnimationController?.controller?.SetParameter("IsUnequipping", true);
                    Log.Trace($"Switch To Weapon: {myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.Name}");
                    myAudioHandler?.Unequip();
                }
            }

            if (input.Contains(PlayerInputType.SwitchToWeaponTwo))
            {
                if (myWeapons[1] != null && myWeapons[1]?.GetScript<Weapon>()?.Behaviour.Id != 0 && myCurrentWeapon != 1)
                {
                    myCurrentWeapon = 1;
                    myIsUnequipping = true;
                    myAnimationController?.controller?.SetParameter("IsUnequipping", true);
                    Log.Trace($"Switch To Weapon: {myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.Name}");
                    myAudioHandler?.Unequip();
                }
            }

            if (!myIsUnequipping && input.Contains(PlayerInputType.Fire))
            {
                if (myIsRunning)
                {
                    StopRunning();
                    myOutOfBreath = true;
                }

                if (myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.ClipAmmo > 0)
                {
                    if (!myHasShotWithNonAutomatic)
                    {
                        if (myWeapons[myCurrentWeapon].GetScript<Weapon>().Shoot(IsAiming))
                        {
                            myAnimationController?.controller?.SetParameter("IsShooting", true);

                            //AUDIO
                            WeaponId id = myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id;
                            uint ammoInClip = myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.ClipAmmo;
                            myAudioHandler?.PlayShoot(id, ammoInClip + 1);
                        }
                    }

                    if (!myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.IsAutomatic)
                    {
                        myHasShotWithNonAutomatic = true;
                    }
                }
                else
                {
                    //PLAY NO AMMO SOUND
                    if (myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.IsReloading == false)
                    {
                        if (Input.IsMousePressed(MouseButton.Left))
                        {
                            WeaponId id = myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id;
                            uint ammoInClip = myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.ClipAmmo;
                            myAudioHandler?.PlayShoot(id, ammoInClip);
                        }
                    }
                }

                if ((myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.MagazineAmmo > 0 &&
                    myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.ClipAmmo <= 0))
                {
                    if (myWeapons[myCurrentWeapon].GetScript<Weapon>().Reload())
                    {
                        myAnimationController?.controller?.SetParameter("IsReloading", true);

                        //AUDIO
                        WeaponId id = myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id;
                        myAudioHandler?.PlayReload(id);
                    }
                }
            }

            if (input.Contains(PlayerInputType.Fire_Released))
            {
                myHasShotWithNonAutomatic = false;

                myAnimationController?.controller?.SetParameter("IsShooting", false);

                Log.Trace("StopFire");
            }

            if (input.Contains(PlayerInputType.Aim))
            {
                Log.Trace("Aim");
                if (myIsRunning)
                {
                    StopRunning();
                }

                IsAiming = true;
                Vision.SetCameraFoV(myFirstPersonCamera, 40);
                myCurrentMaxSpeed = myStats.AimingMovementSpeed;
                myAnimationController?.controller?.SetParameter("IsAiming", true);
                UIManager.Instance.OnCrosshairStateChange(IsAiming);
            }

            if (input.Contains(PlayerInputType.Aim_Released))
            {
                Log.Trace("StopAim");

                IsAiming = false;
                myCurrentMaxSpeed = myStats.WalkMovementSpeed;
                myAnimationController?.controller?.SetParameter("IsAiming", false);
                UIManager.Instance.OnCrosshairStateChange(IsAiming);

                Vision.SetCameraFoV(myFirstPersonCamera, 60);
            }

            if (input.Contains(PlayerInputType.Reload))
            {
                if (myIsRunning)
                {
                    StopRunning();
                    myOutOfBreath = true;
                }

                if (myWeapons[myCurrentWeapon].GetScript<Weapon>().Reload())
                {
                    myAnimationController?.controller?.SetParameter("IsReloading", true);
                    myAnimationController?.controller?.SetParameter("IsStartingToReload", true);

                    WeaponId id = myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Id;
                    myAudioHandler?.PlayReload(id);
                }
            }
        }

        private void Movement(List<PlayerInputType> input)
        {
            // Move
            MoveLogic(input);

            // Jump
            JumpLogic(input);

            // Run
            RunningLogic(input);
        }

        private void MoveLogic(List<PlayerInputType> input)
        {
            myIsMoving = false;

            if (input.Contains(PlayerInputType.MoveForward))
            {
                Vector3 forward = myFirstPersonCamera.forward;
                forward.y = 0;
                forward.Normalize();

                myMoveDirection += forward;
                myIsMoving = true;
            }

            if (input.Contains(PlayerInputType.MoveBackward))
            {
                Vector3 down = -myFirstPersonCamera.forward;
                down.y = 0;
                down.Normalize();

                myMoveDirection += down;

                // Reset breath
                if (myOutOfBreath)
                {
                    myOutOfBreath = false;
                }

                myIsMoving = true;
            }

            if (input.Contains(PlayerInputType.MoveLeft))
            {
                Vector3 left = -myFirstPersonCamera.right;
                left.y = 0;
                left.Normalize();

                myMoveDirection += left;

                myIsMoving = true;

                if (!IsAiming)
                {
                    if (FPSCamera.HasScript<WeaponSway>())
                    {
                        FPSCamera.GetScript<WeaponSway>().SetTargetRotation(new Vector3(0, 0, Mathf.Radians(1f)));
                        FPSCamera.GetScript<WeaponSway>().SetTargetSideSway(-5);
                    }
                }
            }

            if (input.Contains(PlayerInputType.MoveRight))
            {
                Vector3 right = myFirstPersonCamera.right;
                right.y = 0;

                right.Normalize();

                myMoveDirection += right;

                myIsMoving = true;

                if (!IsAiming)
                {
                    if (FPSCamera.HasScript<WeaponSway>())
                    {
                        FPSCamera.GetScript<WeaponSway>().SetTargetRotation(new Vector3(0, 0, Mathf.Radians(-1f)));
                        FPSCamera.GetScript<WeaponSway>().SetTargetSideSway(5);
                    }
                }
            }

            myVelocity += myMoveDirection.Normalized() * myCurrentAcceleration * Time.deltaTime;

            myVelocity.ClampMagnitude(myCurrentMaxSpeed);

            if (!myIsMoving && !myIsJumping)
            {
                myAnimationController?.controller?.SetParameter("IsWalking", false);
                myVelocity = Vector3.Lerp(myVelocity, Vector3.Zero, (myStats.MovementAcceleration / myVelocity.Length()) * Time.deltaTime);
            }
            else
            {
                myAnimationController?.controller?.SetParameter("IsWalking", true);
            }

            myCharacterController.Move(myVelocity * Time.deltaTime);

            myMoveDirection = Vector3.Zero;
        }

        private void JumpLogic(List<PlayerInputType> input)
        {
            if (myCharacterController.isGrounded && input.Contains(PlayerInputType.Jump))
            {
                myIsJumping = true;
                myCurrentAcceleration = myStats.InAirAcceleration;
                myCharacterController.Jump(myJumpForce);
                myAudioHandler?.PlayJump();
            }
        }

        private void RunningLogic(List<PlayerInputType> input)
        {
            // Run Cooldown
            if (myRunCooldown > 0)
            {
                myRunCooldown -= Time.deltaTime;
            }

            // Stamina regeneration
            if (myStamina < myStats.Stamina && !myIsRunning)
            {
                myStamina += myStats.StaminaRegenerationRate * Time.deltaTime;

                if (myStamina > myStats.Stamina)
                {
                    myStamina = myStats.Stamina;
                }
            }

            // Stamina depletion when running
            if (myIsRunning)
            {
                myStamina -= Time.deltaTime;

                if (myStamina < 0)
                {
                    StopRunning();
                    myOutOfBreath = true;
                    myRunCooldown = myStats.RunCooldown;
                }
            }

            // Start Running
            if (!myOutOfBreath && myRunCooldown <= 0 && input.Contains(PlayerInputType.Sprint) && input.Contains(PlayerInputType.MoveForward))
            {
                if (!IsAiming)
                {
                    StartRunning();
                }
            }

            // Stop Running
            if (myIsRunning == true && (input.Contains(PlayerInputType.Sprint_Released) || input.Contains(PlayerInputType.MoveForward_Released)))
            {
                StopRunning();
            }

            // Reset breath
            if (myOutOfBreath && input.Contains(PlayerInputType.Sprint_Released))
            {
                myOutOfBreath = false;
            }
        }

        private void StartRunning()
        {
            myIsRunning = true;
            myCurrentMaxSpeed = myStats.RunningMovementSpeed;
            myAnimationController?.controller?.SetParameter("IsRunning", true);
        }

        private void StopRunning()
        {
            myIsRunning = false;
            myCurrentMaxSpeed = myStats.WalkMovementSpeed;
            myAnimationController?.controller?.SetParameter("IsRunning", false);
        }

        public void TakeDamage()
        {
            if (myIsDead) { return; }

            myShouldRegen = false;

            if (myRegenTimer != null)
            {
                entity.CancelTimer(myRegenTimer);
            }

            myRegenTimer = entity.CreateTimer(3f, () =>
            {
                myShouldRegen = true;
            });

            myStats.CurrentHealth -= 50;

            if (myFirstPersonCamera != null)
            {
                Vision.CameraShakeSettings shakeSettings;
                shakeSettings.shakeTime = 0.3f;
                shakeSettings.rotationMagnitude = 10f;
                shakeSettings.translationMagnitude = 0f;
                shakeSettings.rotationAmount = new Vector3(5, 5, 5);
                shakeSettings.translationAmount = new Vector3(0, 0, 0);

                Vision.DoCameraShake(myFirstPersonCamera, shakeSettings);
            }

            VFXManager.Instance.SetTakeDamagePPFX(myStats.ModifiedMaxHealth, myStats.CurrentHealth);

            if (myStats.CurrentHealth <= 0)
            {
                PlayerDeath();
            }
            else
            {
                myAudioHandler?.TakeDamage((uint)myStats.myPlayerType);
            }

            Log.Info("Player Hit!  HP: " + myStats.CurrentHealth.ToString());
        }

        public void PlayerDeath()
        {
            if (PerkManager.Instance.PlayerPerks.Contains(Perk.Revive))
            {
                UseRevivePerk();
                return;
            }

            myIsDead = true;

            myAudioHandler?.Death((uint)myStats.myPlayerType);

            GameManager.Instance.EndGame();
        }

        public void UseRevivePerk()
        {
            ClearPerks();

            uint layermask = 1 << 2;
            Entity[] hitEnemies = Physics.OverlapSphere(entity.position, 1000, layermask);
            foreach (Entity ent in hitEnemies)
            {
                if (ent != null)
                {
                    if (ent.HasScript<EnemyBase>())
                    {
                        NetEvents.EventFromLocalId(ent.Id, eNetEvent.Death);
                    }
                }
            }

            myStats.CurrentHealth = 1f;

            ReclaculateModifiers();
        }

        public void AddPerk(Perk_Base aPerk)
        {
            myPerks.Add(aPerk);
            ReclaculateModifiers();

            Log.Info("Player Gained Perk!: " + aPerk.ToString());
        }

        public void ClearPerks()
        {
            PerkManager.Instance.ClearPerks();
            myPerks.Clear();
            ReclaculateModifiers();
        }


        public WeaponBehaviour GetCurrentWeapon()
        {
            return myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour != null ? myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour : null;
        }

        public void SetCurrentWeapon(WeaponBehaviour aWeapon)
        {
            if (aWeapon != null)
            {
                if (myWeapons[1] != null && myWeapons[1]?.GetScript<Weapon>()?.Behaviour.Id == 0)
                {
                    myWeapons[1].GetScript<Weapon>().Behaviour = aWeapon;
                    myCurrentWeapon = 1;

                    myIsUnequipping = true;
                    myAnimationController?.controller?.SetParameter("IsUnequipping", true);
                    Log.Trace($"Switch To Weapon: {myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.Name}");
                    myAudioHandler?.Unequip();
                }
                else
                {
                    myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour = aWeapon;

                    myIsUnequipping = true;
                    myAnimationController?.controller?.SetParameter("IsUnequipping", true);
                    Log.Trace($"Switch To Weapon: {myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.Name}");
                    myAudioHandler?.Unequip();
                }
            }
        }

        private void ReclaculateModifiers()
        {
            PlayerStats playerStats = new PlayerStats();
            playerStats.Initialize(myStats.myPlayerType);

            foreach (Perk_Base perk in myPerks)
            {
                perk?.AddModifier(playerStats);
            }

            myStats = playerStats;
        }

        //DEBUG UI

        private void OnRenderUI()
        {
            //if (font == null) { return; }
            //UIRenderer.DrawString("HP: " + myStats.CurrentHealth.ToString(), font, new Vector3(-500f, 300f, 10f), new Vector2(20f), 0f, 100f, new Vector4(1f));
            //UIRenderer.DrawString("ROUND: [" + GameManager.Instance.GetCurrentRound().ToString() + "]", font, new Vector3(-500f, 270f, 10f), new Vector2(20f), 0f, 100f, new Vector4(1f));

            //if (myWeapons[myCurrentWeapon].HasScript<Weapon>() && myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour != null)
            //{
            //    UIRenderer.DrawString("WEAPON: " + myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.Name, font, new Vector3(470f, -270f, 10f), new Vector2(20f), 0f, 100f, new Vector4(1f));
            //    UIRenderer.DrawString("AMMO: [" + myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.ClipAmmo.ToString() + "/" + myWeapons[myCurrentWeapon].GetScript<Weapon>().Behaviour.Information.MagazineAmmo + "]", font, new Vector3(470f, -300f, 10f), new Vector2(20f), 0f, 100f, new Vector4(1f));
            //}

            //UIRenderer.DrawString("TOTAL POINTS: " + PointManager.Instance.totalPoints, font, new Vector3(470f, -150f, 10f), new Vector2(20f), 0f, 100f, new Vector4(1f));
            //UIRenderer.DrawString("CURRENT POINTS: " + PointManager.Instance.currentPoints, font, new Vector3(470f, -100f, 10f), new Vector2(20f), 0f, 100f, new Vector4(1f));

            //UIRenderer.DrawSprite(new Vector3(0f, 0f, 10f), new Vector2(4f), 0f, new Vector4(0f, 0f, 0f, 1f));
        }

        //DEBUG UI

        private bool myIsJumping;
        public bool IsJumping
        {
            get { return myIsJumping; }
        }
        private void LandLogic()
        {
            if (myIsJumping && myCharacterController.isGrounded)
            {
                if (FPSCamera.HasScript<WeaponSway>())
                {
                    FPSCamera.GetScript<WeaponSway>().SetTargetBounce(10);
                }

                myIsJumping = false;
                myCurrentAcceleration = myStats.MovementAcceleration;
                myAudioHandler?.PlayLand();
            }
        }

        private void OnPause(UIManager.UIState aState)
        {
            if(aState == UIManager.UIState.HUD)
            {
                myIsPaused = false;
            }
            else
            {
                myIsPaused = true;
            }
        }
    }
}
