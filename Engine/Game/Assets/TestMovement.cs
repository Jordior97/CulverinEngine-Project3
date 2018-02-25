﻿using CulverinEditor;
using CulverinEditor.Debug;
using CulverinEditor.Map;

//Attach this script to the tank parent object if you want to see it rotate
public class TestMovement : CulverinBehaviour
{
    public enum Direction
    {
        NORTH = 0,
        EAST,
        SOUTH,
        WEAST
    }


    public Direction curr_dir = Direction.NORTH;
    public int start_direction = 1;
    private float movSpeed = 0.0f;
    Vector3 endPosition;
    Vector3 endRotation;
    public float distanceToMove = 10.0f;
    int[,] array2Da;
    public int curr_x = 0;
    public int curr_y = 0;
    int map_width = 0;
    int map_height = 0;
    public bool blocked_camera = false;

    private int angle = 0;
    private float speed_rotation = 30;
    private float actual_angle = 0;
    private bool rotating = false;

    private CompAudio audio;

    //2D coordinates, y=z in 3D coordinates

    void Start()
    {

        audio = GetComponent<CompAudio>();
        audio.PlayEvent("PlayMusic");
        curr_dir = (Direction)start_direction;
        map_width = Map.GetWidthMap();
        map_height = Map.GetHeightMap();
        array2Da = new int[map_width, map_height];
        for (int y = 0; y < map_height; y++)
        {
            for (int x = 0; x < map_width; x++)
            {
                array2Da[x, y] = 0;
            }
        }
        //array2Da[0,0] = 1;
        //Debug.Log(array2Da[0,0].ToString());
        endPosition = GetComponent<Transform>().local_position;
        endRotation = transform.local_rotation;

        string map = Map.GetMapString();
        Debug.Log(map);
        int t = 0;
        for (int y = 0; y < map_height; y++)
        {
            for (int x = 0; x < map_width; x++)
            {
                array2Da[x, y] = int.Parse(map[t].ToString());
                t += 1;
            }
        }

        //Search player position
        for (int y = 0; y < map_height; y++)
        {
            for (int x = 0; x < map_width; x++)
            {
                if (array2Da[x, y] == 2)
                {
                    curr_x = x;
                    curr_y = y;
                    array2Da[x, y] = 0;
                    MovePositionInitial(new Vector3((float)curr_x * distanceToMove, GetComponent<Transform>().position.y, (float)curr_y * distanceToMove));
                }
            }
        }
    }

    void Update()
    {
        int tile_mov_x = 0;
        int tile_mov_y = 0;
        start_direction = (int)curr_dir;

        //if (Input.GetMouseButtonRepeat(2) && !blocked_camera)
        //{
        //    Debug.Log("Trying to rotate");
        //    float rot_x = Input.GetMouseYAxis();
        //    float rot_y = Input.GetMouseXAxis();

        //    if (rot_x != 0)
        //    {
        //        transform.local_rotation = new Vector3(transform.local_rotation.x - rot_x, transform.local_rotation.y, 0);
        //    }

        //    if (rot_y != 0)
        //    {
        //        transform.local_rotation = new Vector3(transform.local_rotation.x, transform.local_rotation.y + rot_y, 0);
        //    }
        //}
        //else
        //{
        //    //Quaternion rot_step = Quaternion.RotateTowards(Quaternion.FromEulerAngles(transform.rotation), Quaternion.FromEulerAngles(endRotation), movSpeed * 20 * Time.DeltaTime());
        //    //transform.local_rotation = rot_step.ToEulerAngles();
        //}
        //if (Input.GetMouseButtonUp(2))
        //{
        //    Vector3 rot_north = Vector3.Zero;
        //    Vector3 rot_east = new Vector3(0, 90, 0);
        //    Vector3 rot_south = new Vector3(0, 180, 0);
        //    Vector3 rot_weast = new Vector3(0, 270, 0);

        //    float curr_rot_y = transform.local_rotation.y;

        //    if (curr_rot_y < 45 || curr_rot_y > 315)
        //    {
        //        endRotation = rot_north;
        //        //curr_dir = Direction.NORTH;
        //    }
        //    if (curr_rot_y > 45 && curr_rot_y < 135)
        //    {
        //        endRotation = rot_east;
        //        //curr_dir = Direction.EAST;
        //    }
        //    if (curr_rot_y > 135 && curr_rot_y < 225)
        //    {
        //        endRotation = rot_south;
        //        //curr_dir = Direction.SOUTH;
        //    }
        //    if (curr_rot_y > 255 && curr_rot_y < 315)
        //    {
        //        endRotation = rot_weast;
        //        //curr_dir = Direction.WEAST;
        //    }
        //}

        if (GetComponent<Transform>().local_position == endPosition && rotating == false)
        {

            if (Input.GetKeyDown(KeyCode.Q)) //Left
            {
                actual_angle = 0;
                angle = -10;
                rotating = true;
                ModificateCurrentDirection(true);

            }
            if (Input.GetKeyDown(KeyCode.E)) //Right
            {
                actual_angle = 0;
                angle = 10;
                rotating = true;
                ModificateCurrentDirection(false);
            }

            if (Input.GetKeyDown(KeyCode.A)) //Left
            {
                audio.PlayEvent("Footsteps");
                MoveLeft(out tile_mov_x, out tile_mov_y);
            }
            else if (Input.GetKeyDown(KeyCode.D)) //Right
            {
                audio.PlayEvent("Footsteps");
                MoveRight(out tile_mov_x, out tile_mov_y);
            }
            else if (Input.GetKeyDown(KeyCode.W)) //Up
            {
                audio.PlayEvent("Footsteps");
                MoveForward(out tile_mov_x, out tile_mov_y);
            }
            else if (Input.GetKeyDown(KeyCode.S)) //Down
            {
                audio.PlayEvent("Footsteps");
                MoveBackward(out tile_mov_x, out tile_mov_y);
            }

            //Calculate endPosition
            if ((tile_mov_x != 0 || tile_mov_y != 0) && array2Da[curr_x + tile_mov_x, curr_y + tile_mov_y] == 0)
            {
                endPosition = new Vector3(GetComponent<Transform>().local_position.x + distanceToMove * (float)tile_mov_x, GetComponent<Transform>().local_position.y, GetComponent<Transform>().local_position.z + distanceToMove * (float)tile_mov_y);
                curr_x += tile_mov_x;
                curr_y += tile_mov_y;
            }
            else
            {
                //Debug.Log("Failed to move");
                //Debug.Log(tile_mov_x.ToString());
                //Debug.Log(tile_mov_y.ToString());
            }
        }
        else if (rotating)
        {
            transform.RotateAroundAxis(Vector3.Up, angle * speed_rotation * Time.DeltaTime());
            float moved_angle = (float)angle * speed_rotation * Time.DeltaTime();
            if (angle < 0)
            {
                actual_angle += (moved_angle * -1);
            }
            else
            {
                actual_angle += moved_angle;
            }

            if (actual_angle >= 90)
            {
                rotating = false;
                if(actual_angle > 90)
                {
                    float marge = actual_angle - 90;
                    if (angle < 0)
                    {
                        transform.RotateAroundAxis(Vector3.Up, marge);
                    }
                    else
                    {
                        transform.RotateAroundAxis(Vector3.Up, -marge);
                    }
                }
            }

        }
        else
        {
            GetComponent<Transform>().local_position = Vector3.MoveTowards(GetComponent<Transform>().local_position, endPosition, movSpeed * Time.DeltaTime());
        }
    }

    public void BlockCamera()
    {
        blocked_camera = true;
    }

    public void UnBlockCamera()
    {
        blocked_camera = false;
    }

    void MovePositionInitial(Vector3 newpos)
    {
        GetComponent<Transform>().SetPosition(newpos);
    }

    void ModificateCurrentDirection(bool left)
    {
        if (left)
        {
            if (curr_dir == Direction.NORTH)
            {
                curr_dir = Direction.WEAST;
            }
            else if (curr_dir == Direction.EAST)
            {
                curr_dir = Direction.NORTH;
            }
            else if (curr_dir == Direction.SOUTH)
            {
                curr_dir = Direction.EAST;
            }
            else if (curr_dir == Direction.WEAST)
            {
                curr_dir = Direction.SOUTH;
            }
        }
        else
        {
            if (curr_dir == Direction.NORTH)
            {
                curr_dir = Direction.EAST;
            }
            else if (curr_dir == Direction.EAST)
            {
                curr_dir = Direction.SOUTH;
            }
            else if (curr_dir == Direction.SOUTH)
            {
                curr_dir = Direction.WEAST;
            }
            else if (curr_dir == Direction.WEAST)
            {
                curr_dir = Direction.NORTH;
            }
        }
    }

    public void MoveForward(out int tile_mov_x, out int tile_mov_y)
    {
        tile_mov_x = 0;
        tile_mov_y = 0;
        if (curr_dir == Direction.NORTH)
        {
            tile_mov_y = -1;
        }
        if (curr_dir == Direction.EAST)
        {
            tile_mov_x = 1;
        }
        if (curr_dir == Direction.SOUTH)
        {
            tile_mov_y = 1;
        }
        if (curr_dir == Direction.WEAST)
        {
            tile_mov_x = -1;
        }
    }

    public void MoveRight(out int tile_mov_x, out int tile_mov_y)
    {
        tile_mov_x = 0;
        tile_mov_y = 0;
        if (curr_dir == Direction.NORTH)
        {
            tile_mov_x = 1;
        }
        if (curr_dir == Direction.EAST)
        {
            tile_mov_y = 1;
        }
        if (curr_dir == Direction.SOUTH)
        {
            tile_mov_x = -1;
        }
        if (curr_dir == Direction.WEAST)
        {
            tile_mov_y = -1;
        }
    }

    public void MoveBackward(out int tile_mov_x, out int tile_mov_y)
    {
        tile_mov_x = 0;
        tile_mov_y = 0;
        if (curr_dir == Direction.NORTH)
        {
            tile_mov_y = 1;
        }
        if (curr_dir == Direction.EAST)
        {
            tile_mov_x = -1;
        }
        if (curr_dir == Direction.SOUTH)
        {
            tile_mov_y = -1;
        }
        if (curr_dir == Direction.WEAST)
        {
            tile_mov_x = 1;
        }
    }

    public void MoveLeft(out int tile_mov_x, out int tile_mov_y)
    {
        tile_mov_x = 0;
        tile_mov_y = 0;
        if (curr_dir == Direction.NORTH)
        {
            tile_mov_x = -1;
        }
        if (curr_dir == Direction.EAST)
        {
            tile_mov_y = -1;
        }
        if (curr_dir == Direction.SOUTH)
        {
            tile_mov_x = 1;
        }
        if (curr_dir == Direction.WEAST)
        {
            tile_mov_y = 1;
        }
    }

}