/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microsh.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bde-carv <bde-carv@student.42wolfsburg.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/06 18:52:31 by bde-carv          #+#    #+#             */
/*   Updated: 2022/12/09 15:49:55 by bde-carv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_write(char *err_msg, char *arg)
{
	write(STDERR_FILENO, err_msg, ft_strlen(err_msg));
	write(STDERR_FILENO, arg, ft_strlen(arg));
	write(1, "\n", 1);
}

void ft_close(int *fd)
{
	if (*fd != -1)
		close(*fd);
	*fd = -1;
}

int	main(int ac, char **av, char **envp)
{
	int	i;
	int	pid;
	int	first_cmd;
	int	fd_in[2] = {-1, -1};
	int	fd_out[2] = {-1, -1};

	i = 0;
	pid = 0;
	first_cmd = 1;

	if (ac < 2)
		return (0);
	
	while (av[i])
	{
		i++;
		while (av[i] && av[i][0] == ';')
			i++;
		if (!av[i])
			break;
		
		if (strcmp(av[i], "cd") == 0)
		{
			first_cmd = i; // position of cd
			while (av[i] && av[i][0] != ';') /* check nr of args of cd. must be 2 (= cd + path)*/
				i++;
			if (i - first_cmd != 2)
				ft_write("error: cd: bad arguments", "");
			else if (chdir(av[first_cmd + 1]) == -1)
				ft_write("error: cd: cannot change directory to ", av[first_cmd + 1]);
		}
		else
		{
			first_cmd = i;
			i++;

			while (av[i])
			{
				if (av[i][0] == '|') // go through until | or ; is found which marks a cmd to execute
				{
					if (pipe (fd_out) == -1) // creating pipe
						ft_write("error: fatal", "");
					break;
				}
				else if (av[i][0] == ';' && !av[i][1])
					break;
				i++;
			}
			pid = fork();
			if (pid == 0) // pid= 0 means we are in child, pid != 0 means parent process
			{
				if (fd_out[1] != -1)
				{
					close(fd_out[0]);
					dup2(fd_out[1], STDOUT_FILENO); // stdout(terminal) now refers to fd_out[1] (output of the command);
					close(fd_out[1]);
				}
				if (fd_in[1] != -1)
				{
					close(fd_in[0]);
					dup2(fd_out[0], STDIN_FILENO);
					close(fd_in[0]);
				}
				av[i] = NULL; // temporarily nullterminating the part (=current cmd to execute) for execve. is necessary for execve that the string is 0 temrinated.
				if (execve(av[first_cmd], &av[first_cmd], envp) == -1)
					ft_write("error: cannot execute ", av[first_cmd]);
				exit (0);
			}
			else if (pid == -1)
				ft_write("error: fatal", "");
			ft_close(&fd_out[0]);
			ft_close(&fd_out[1]);
			fd_in[0] = fd_out[0];
			fd_in[1] = fd_out[1];
			fd_out[0] = -1;
			fd_out[1] = -1;
			if (pid > 0) // parent waiting for child to terminate
				waitpid(pid, NULL, WUNTRACED);
		}
	}
	return (0);
}
